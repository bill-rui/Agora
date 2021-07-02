pipeline{
	agent none
	stages{
		stage('Run Build'){
			parallel{
				stage('build on controller'){
					agent {label 'Falcon'}
					steps{
						sh 'source /opt/intel/compilers_and_libraries_2020.3.279/linux/bin/compilervars.sh intel64'
						sh './test/jenkins_test/build_ue.sh'					
					}
				}				
			}
		}
	}
}