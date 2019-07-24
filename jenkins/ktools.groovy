
node {
    hasFailed = false
    //sh 'sudo /var/lib/jenkins/jenkins-chown'
    deleteDir() // wipe out the workspace

    properties([
      parameters([
        [$class: 'StringParameterDefinition',  name: 'KTOOLS_BRANCH', defaultValue: BRANCH_NAME],
        [$class: 'StringParameterDefinition',  name: 'RELEASE_TAG', defaultValue: "${BRANCH_NAME}-${BUILD_NUMBER}"],
        [$class: 'StringParameterDefinition',  name: 'TEST_GCC', defaultValue: "8 7 6"],
        [$class: 'BooleanParameterDefinition', name: 'PURGE', defaultValue: Boolean.valueOf(true)],
        [$class: 'BooleanParameterDefinition', name: 'PUBLISH', defaultValue: Boolean.valueOf(false)],
        [$class: 'BooleanParameterDefinition', name: 'SLACK_MESSAGE', defaultValue: Boolean.valueOf(false)]
      ])
    ])

    // Model vars
    String ktools_branch    = params.KTOOLS_BRANCH
    String ktools_git_url   = "git@github.com:OasisLMF/Ktools.git"
    String ktools_workspace = 'ktools_workspace'
    String git_creds = "1335b248-336a-47a9-b0f6-9f7314d6f1f4"

    // Set Global ENV
    env.KTOOLS_IMAGE_GCC   = "jenkins/Dockerfile.build-ktools"   // Docker image for testing gcc build
    env.KTOOLS_IMAGE_CLANG = "jenkins/Dockerfile.clang-build"    // Docker image for building static build of ktools

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
        if (params.PUBLISH){
            stage("Ktools Builder: CLANG") {
                dir(ktools_workspace) {
                    //  Build Static TAR using clang
                    sh "docker build -f $env.KTOOLS_IMAGE_CLANG -t ktools-builder ."
                    sh 'docker run -v $(pwd):/var/ktools ktools-builder"
                    
                    //Archive TAR to CI
                    archiveArtifacts artifacts: 'tar/**/*.*'
                }
            }
        // Test dynamic linked builds GCC 
        } else {
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
        }
    } catch(hudson.AbortException | org.jenkinsci.plugins.workflow.steps.FlowInterruptedException buildException) {
        hasFailed = true
        error('Build Failed')
    } finally {
        dir(ktools_workspace) {
            //Run house cleaning (if needed)
        }
        //Notify on slack -- Needs fixing
        if(params.SLACK_MESSAGE && (params.PUBLISH || hasFailed)){
            // def slackColor = hasFailed ? '#FF0000' : '#27AE60'
            // SLACK_GIT_URL = "https://github.com/OasisLMF/${model_name}/tree/${model_branch}"
            // SLACK_MSG = "*${env.JOB_NAME}* - (<${env.BUILD_URL}|${env.RELEASE_TAG}>): " + (hasFailed ? 'FAILED' : 'PASSED')
            // SLACK_MSG += "\nBranch: <${SLACK_GIT_URL}|${model_branch}>"
            // SLACK_MSG += "\nMode: " + (params.PUBLISH ? 'Publish' : 'Build Test')
            // SLACK_CHAN = (params.PUBLISH ? "#builds-release":"#builds-dev")
            // slackSend(channel: SLACK_CHAN, message: SLACK_MSG, color: slackColor)
        }

        // Handle JOB Publish
        // TODO: append to CHANGELOG
        if(! hasFailed && params.PUBLISH){
            sshagent (credentials: [git_creds]) {
                dir(ktools_workspace) {
                    sh "git tag ${RELEASE_TAG}"
                    sh "git  push origin ${RELEASE_TAG}"

                    //POST TAR TO GH RELEASE
                    //  -- Hook into github API and upload tar.gz to matching tag
                }
            }
        }
    }
}
