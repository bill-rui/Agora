pipeline{
	agent none
	stages{
		stage('Build'){
			parallel{
				stage('build on sender'){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/sim/build.sh -UE'
						dir('./data'){
							stash includes: './data/LDPC_orig_ul_data_2048_ant8.bin', name: 'data_file_1'
							stash includes: './data/LDPC_rx_data_2048_ant8.bin', name: 'data_file_2'
							stash includes: './data/LDPC_orig_dl_data_2048_ant8.bin', name: 'data_file_3'
							stash includes: './data/LDPC_dl_tx_data_2048_ant8.bin', name: 'data_file_4'
						}					
					}
				}
				stage("build on receiver"){
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
					unstash 'data_file_1'
					unstash 'data_file_2'
					unstash 'data_file_3'
					unstash 'data_file_4'
				}	
			}
		}
		stage('start radios'){
			parallel{
				stage('start sender'){
					agent{label 'Harrier'}
					steps{
						sleep 2  // wait for receiver to start first
						sh './test/jenkins_test/start_ue.sh'
					}	
				}
				stage('start BS'){
					agent{label 'Falcon'}
					steps{
						sh './test/jenkins_test/start_bs.sh'
					}
				}
			}
		}
	}
}