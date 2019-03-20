
node {
    hasFailed = false
    //sh 'sudo /var/lib/jenkins/jenkins-chown'
    deleteDir() // wipe out the workspace

    properties([
      parameters([
        [$class: 'StringParameterDefinition',  name: 'KTOOLS_BRANCH', defaultValue: BRANCH_NAME],
        [$class: 'StringParameterDefinition',  name: 'RELEASE_TAG', defaultValue: "${BRANCH_NAME}-${BUILD_NUMBER}"],
        [$class: 'StringParameterDefinition',  name: 'TEST_GCC', defaultValue: "8 7 6 5"],
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
    env.KTOOLS_IMAGE     = "jenkins/Dockerfile.build-ktools"                     // Docker image for worker

    try {
        stage('Clone: Ktools') {
            sshagent (credentials: [git_creds]) {
                dir(ktools_workspace) {
                    sh "git clone --recursive ${ktools_git_url} ."
                    if (ktools_branch.matches("PR-[0-9]+")){
                        // Checkout PR and merge into target branch, test on the result
                        sh "git fetch origin pull/$CHANGE_ID/head:$BRANCH_NAME"
                        sh "git checkout $BRANCH_NAME"
                        sh "git format-patch $CHANGE_TARGET --stdout > ${BRANCH_NAME}.patch"
                        sh "git checkout $CHANGE_TARGET"
                        sh "git apply --stat ${BRANCH_NAME}.patch"  // Print files changed
                        sh "git apply --check ${BRANCH_NAME}.patch" // Check for merge conflicts
                        sh "git apply ${BRANCH_NAME}.patch"         // Apply the patch
                    } else {
                        // Checkout branch
                        sh "git checkout ${ktools_branch}"
                    }
                }
            }
        }
        stage('Shell Env'){
            // Print Build Params here 
            sh "env"
        }
        
        gcc_vers = params.TEST_GCC.split()
        for(int i=0; i < gcc_vers.size(); i++) {
            stage("Ktools Tester: GCC: ${gcc_vers[i]}") {
                dir(ktools_workspace) {
                    // Build & run ktools testing image
                    sh "sed -i 's/FROM.*/FROM gcc:${gcc_vers[i]}/g' $env.KTOOLS_IMAGE"
                    sh "docker build -f $env.KTOOLS_IMAGE -t ktools-runner:${gcc_vers[i]} ."
                    sh "docker run ktools-runner:${gcc_vers[i]}"
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
                    sh PIPELINE + " git_tag ${env.TAG_RELEASE}"
                }
            }
        }
        //Store logs
        dir(ktools_workspace) {
            // Store build files here 

            //archiveArtifacts artifacts: 'stage/log/**/*.*', excludes: '*stage/log/**/*.gitkeep'
            //archiveArtifacts artifacts: "stage/output/**/*.*"
        }
    }
}
