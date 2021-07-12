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
						sleep 2  // wait for receiver to start first
						sh './test/jenkins_test/sim/start_ue.sh'
					}	
				}
				stage('start BS'){
					agent{label 'Falcon'}
					steps{
						sh './test/jenkins_test/sim/start_bs.sh'
					}
				}
			}
		}
	}
}