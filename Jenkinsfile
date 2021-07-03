pipeline{
	agent none
	stages{
		stage{
			parallel{
				stage('build on controller'){
					agent {label 'Falcon'}
					steps{
						sh './test/jenkins_test/build_ue.sh'					
					}
				}
				stage("build on agent"){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/build_ue.sh'
					}
				}				
			}
			
			stage{
					agent {label 'Falcon'}
					dir('./data'){
						stash includes: 'orig_data_512_ant2.bin', name 'data_file'
					}
					agent {label 'Harrier'}
					dir('./data'){
						unstash 'data_file'
					}
				}
		}
	}
}