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
							sh 'echo "hello"'
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
			stage('copy data file'){
				agent {label 'Harrier'}
				steps{
					dir('./data'){
						//unstash 'data_file'
					}
				}
				
			}
		}
	}
}