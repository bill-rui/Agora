pipeline{
	agent none
	stages{
		stage('Build'){
			parallel{
				stage('build on controller'){
					agent {label 'Falcon'}
					steps{
						sh './test/jenkins_test/build_ue.sh'
						dir('./data'){
							stash 'orig_data_512_ant2.bin'
						}					
					}
				}
				stage("build on agent"){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/build_bs.sh'
					}
				}				
			}
			node('Harrier'){
				dir('./data'){
					unstash 'orig_data_512_ant2.bin'
				}
			}
		}
	}
}