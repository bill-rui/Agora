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
							stash includes: 'orig_data_512_ant2.bin', name: 'repo_root'						
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
					unstash 'data_file'
				}	
			}
		}
	}
}