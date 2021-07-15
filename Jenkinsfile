/**
* Jenkinsfile for running tests with emulated hardware. Uses json file tddconfig-sim-ul.json to run sumulated
* sender and agora on Harrier and Falcon respectively.
* requires build.sh, start_bs.sh, start_ue.sh in ./test/jenkins_test/sim directory
**/

pipeline{
	agent none
	stages{
		stage('Build'){  // build agora on Harrier and Falcon simultaneously
			parallel{
				stage('build on sender'){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/sim/build.sh -UE'
						dir('./data'){  // copy the data files on Harrier (sender)
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
						sh 'sudo ./test/jenkins_test/sim/start_ue.sh'
					}
				}
				stage('start receiver'){
					agent{label 'Falcon'}
					steps{
						dir('./data'){  // paste the data files stashed earlier
							unstash 'ul_orig'
							unstash 'dl_orig'
							unstash 'rx'
							unstash 'tx'
						}
						sh 'sudo ./test/jenkins_test/sim/start_bs.sh'
					}
				}
			}
		}
		stage('BER test'){
			agent{label 'Falcon'}
			steps{
				sh 'python3 ./test/jenkins_test/sim/compare_values.py 0.01'  // check the BER values, fail threshold being 0.01
			}
		}
	}
}