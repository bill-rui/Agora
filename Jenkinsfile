pipeline{
	agent none
	stages{
		stage('Build'){
			parallel{
				stage('build on sender'){
					agent {label 'Harrier'}
					steps{
						sh './test/jenkins_test/sim/build.sh -UE'				
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
						sleep 3  // wait for receiver to start first
						sh './test/jenkins_test/sim/start_ue.sh'
					}	
				}
				stage('start receiver'){
					agent{label 'Falcon'}
					steps{
						sh 'pwd'
						sh './test/jenkins_test/sim/start_bs.sh'
						
					}
				}
			}
		}
	}
}