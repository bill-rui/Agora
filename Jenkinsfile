pipeline{
	agent none
	stages{
		stage('Run Build'){
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
		}
	}
}