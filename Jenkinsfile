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
							stash includes: 'LDPC_orig_ul_data_2048_ant8.bin', name: 'ul_orig'
							stash includes: 'LDPC_rx_data_2048_ant8.bin', name: 'rx'
							stash includes:  'LDPC_orig_dl_data_2048_ant8.bin', name: 'dl_orig'
							stash includes:  'LDPC_dl_tx_data_2048_ant8.bin', name: 'tx'
						}
					}
				}
				stage("build on receiver"){
					agent {label 'Falcon'}
					steps{
						sh './test/jenkins_test/sim/build.sh -BS'
					}
				}				
			}
		}
		stage('start radios'){
			parallel{
				stage('start sender'){
					agent{label 'Harrier'}
					steps{
						sleep 2  // wait for receiver to start first
						sh './test/jenkins_test/sim/start_ue.sh'
					}
				}
				stage('start receiver'){
					agent{label 'Falcon'}
					steps{
						dir('./data'){
							unstash 'ul_orig'
							unstash 'dl_orig'
							unstash 'rx'
							unstash 'tx'
						}
						sh './test/jenkins_test/sim/start_bs.sh'						
					}
				}
			}
		}
	}
}