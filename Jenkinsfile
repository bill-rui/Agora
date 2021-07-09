pipeline{
	agent none
	stages{
		stage('Build'){
			parallel{
				stage('build on UE'){
					agent {label 'Falcon'}
					steps{
						sh './test/jenkins_test/build_ue.sh'
						dir('./data'){
							stash includes: 'LDPC_orig_dl_data_512_ant1.bin', name: 'dl_file'
							stash includes: 'LDPC_orig_ul_data_512_ant1.bin', name: 'ul_file'
						}					
					}
				}
				stage("build on BS"){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/build_bs.sh'
					}
				}				
			}
		}
		stage('copy data file'){
			agent {label 'Harrier'}
			steps{
				dir('./data'){
					unstash 'dl_file'
					unstash 'ul_file'
				}	
			}
		}
		stage('start radios'){
			environment{
				UE_STARTED = 'false'
				BS_TERMINATED = 'false'
			}
			parallel{
				stage('start UE'){
					agent{label 'Falcon'}
					steps{
						sh '''#!/bin/bash
						. test/jenkins_test/start_radio.sh -UE
						'''
					}	
				}
				stage('start BS'){
					agent{label 'Harrier'}
					steps{
						sh '''#!/bin/bash
						. test/jenkins_test/start_radio.sh -BS
						'''
					}					
				}
			}
		}
	}
}