node {
    hasFailed = false
    sh 'sudo /var/lib/jenkins/jenkins-chown'
    deleteDir() // wipe out the workspace

    properties([
      parameters([
        [$class: 'StringParameterDefinition',  name: 'KTOOLS_BRANCH', defaultValue: BRANCH_NAME],
        [$class: 'StringParameterDefinition',  name: 'RELEASE_TAG', defaultValue: "${BRANCH_NAME}-${BUILD_NUMBER}"],
        [$class: 'StringParameterDefinition',  name: 'TEST_GCC', defaultValue: ""],
        [$class: 'BooleanParameterDefinition', name: 'PURGE', defaultValue: Boolean.valueOf(true)],
        [$class: 'BooleanParameterDefinition', name: 'PUBLISH', defaultValue: Boolean.valueOf(false)],
        [$class: 'BooleanParameterDefinition', name: 'PRE_RELEASE', defaultValue: Boolean.valueOf(true)],
        [$class: 'BooleanParameterDefinition', name: 'SLACK_MESSAGE', defaultValue: Boolean.valueOf(false)]
      ])
    ])

    // Model vars
    String ktools_branch    = params.KTOOLS_BRANCH
    String ktools_git_url   = "git@github.com:OasisLMF/Ktools.git"
    String ktools_workspace = 'ktools_workspace'
    String git_creds = "1335b248-336a-47a9-b0f6-9f7314d6f1f4"

    // Set Global ENV
    env.KTOOLS_IMAGE_GCC   = "jenkins/Dockerfile.gcc-build"    // Docker image for Dynamic linked build
    env.KTOOLS_IMAGE_CLANG = "jenkins/Dockerfile.clang-build"  // Docker image for static ktools build

    //make sure release candidate versions are tagged correctly                                                                              
    if (params.PUBLISH && params.PRE_RELEASE && ! params.RELEASE_TAG.matches('^v(\\d+\\.)(\\d+\\.)(\\*|\\d+)-rc(\\d+)$')) {
        sh "echo release candidates must be tagged v{version}rc{N}, example: v1.0.0rc1"
        sh "exit 1"
    }

    try {
        stage('Clone: Ktools') {
            sshagent (credentials: [git_creds]) {
                dir(ktools_workspace) {
                    sh "git clone --recursive ${ktools_git_url} ."
                    if (ktools_branch.matches("PR-[0-9]+")){
                        // Checkout PR and merge into target branch, test on the result
                        sh "git fetch origin pull/$CHANGE_ID/head:$BRANCH_NAME"
                        sh "git checkout $CHANGE_TARGET"
                        sh "git merge $BRANCH_NAME"
                    } else {
                        // Checkout branch
                        sh "git checkout ${ktools_branch}"
                    }
                }
            }
        }
        // Print Build Params here 
        stage('Shell Env'){
            sh "env"
        }
    
        // Create static build CLANG 
        stage("Ktools Builder: Linux_x86_64") {
            dir(ktools_workspace) {
                sh "docker build -f $env.KTOOLS_IMAGE_CLANG -t ktools-builder ."
            }
        }
        stage("Build: Autotools"){
            dir(ktools_workspace) {
                sh 'docker run --entrypoint build-autotools.sh -v $(pwd):/var/ktools ktools-builder'
                archiveArtifacts artifacts: 'tar/**/*.*'
            }
        }
        stage("Build: Cmake"){
            dir(ktools_workspace) {
                sh 'docker run --entrypoint build-cmake.sh -v $(pwd):/var/ktools ktools-builder'
                archiveArtifacts artifacts: 'tar/**/*.*'
            }
        }

        // Optional: test dynamic linked builds GCC 
        gcc_vers = params.TEST_GCC.split()
        for(int i=0; i < gcc_vers.size(); i++) {
            stage("Ktools Tester: GCC ${gcc_vers[i]}") {
                dir(ktools_workspace) {
                    // Build & run ktools testing image
                    sh "sed -i 's/FROM.*/FROM gcc:${gcc_vers[i]}/g' $env.KTOOLS_IMAGE_GCC"
                    sh "docker build -f $env.KTOOLS_IMAGE_GCC -t ktools-runner:${gcc_vers[i]} ."
                    sh "docker run ktools-runner:${gcc_vers[i]}"
                }
            }
        }

        if (params.PUBLISH){
            // Tag and release 
            stage("Create Release: GitHub") {
                dir(ktools_workspace) {
                    sshagent (credentials: [git_creds]) {
                        sh "git tag ${RELEASE_TAG}"
                        sh "git  push origin ${RELEASE_TAG}"
                    }
                    
                    withCredentials([string(credentialsId: 'github-api-token', variable: 'gh_token')]) {
                        String repo = "OasisLMF/ktools"

                        def json_request = readJSON text: '{}'             
                        json_request['tag_name'] = RELEASE_TAG
                        json_request['target_commitish'] = 'master'
                        json_request['name'] = RELEASE_TAG
                        json_request['body'] = ""
                        json_request['draft'] = false
                        json_request['prerelease'] = params.PRE_RELEASE
                        writeJSON file: 'gh_request.json', json: json_request
                        sh 'curl -XPOST -H "Authorization:token ' + gh_token + "\" --data @gh_request.json https://api.github.com/repos/$repo/releases > gh_response.json"

                        // Fetch release ID and post build tar
                        def response = readJSON file: "gh_response.json" 
                        release_id = response['id']
                        dir('tar') {
                            filename='Linux_x86_64.tar.gz'
                            sh 'curl -XPOST -H "Authorization:token ' + gh_token + '" -H "Content-Type:application/octet-stream" --data-binary @' + filename + " https://uploads.github.com/repos/$repo/releases/$release_id/assets?name=" + filename
                        }    
                    }
                }
            }
        } 
    } catch(hudson.AbortException | org.jenkinsci.plugins.workflow.steps.FlowInterruptedException buildException) {
        hasFailed = true
        error('Build Failed')
    } finally {
        // Run merge back if publish 
        if (params.PUBLISH){ 
            dir(ktools_workspace) {
                sshagent (credentials: [git_creds]) {
                    sh "git checkout master && git pull"
                    sh "git merge ${ktools_branch} && git push"
                    sh "git checkout develop && git pull"
                    sh "git merge master && git push"
                }
            }
        }
    }
}
