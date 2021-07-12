pipeline{
	agent none
	stages{
		stage('Build'){
			parallel{
				stage('build on UE'){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/build_ue.sh'
						dir('./data'){
							stash includes: 'LDPC_orig_dl_data_512_ant1.bin', name: 'dl_file'
							stash includes: 'LDPC_orig_ul_data_512_ant1.bin', name: 'ul_file'
						}					
					}
				}
				stage("build on BS"){
					agent {label 'Falcon'}
					steps{
						sh './test/jenkins_test/build_bs.sh'
					}
				}				
			}
		}
		stage('copy data file'){
			agent {label 'Falcon'}
			steps{
				dir('./data'){
					unstash 'dl_file'
					unstash 'ul_file'
				}	
			}
		}
		stage('start radios'){
			parallel{
				stage('start UE'){
					agent{label 'Harrier'}
					steps{
						script{
							env.UE_STARTED = 'false'
							env.BS_TERMINATED = 'false'
						}
						sh './test/jenkins_test/start_ue.sh'
						script{
							env.UE_STARTED = 'true'
						}
					}	
				}
				stage('start BS'){
					agent{label 'Falcon'}
					steps{
						script{
							while(env.UE_STARTED == 'false'){
								sleep 1
							}
						}
						sh './test/jenkins_test/start_bs.sh'
					}		
				}
			}
		}
	}
}