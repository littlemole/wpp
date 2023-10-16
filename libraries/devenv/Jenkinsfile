pipeline {
    agent any

    stages {
        stage('Build g++') {
            steps {
                echo 'Building devenv ... g++ make '
		sh 'make image'
            }
	}
        stage('Build g++ cmake') {
            steps {
                echo 'Building devenv ... g++ cmake '
		sh 'make image BUILDCHAIN=cmake'
            }
	}
        stage('Build clang') {
            steps {
                echo 'Building devenv ... clang++ make '
		sh 'make image CXX=clang++'
            }
	}
        stage('Build clang cmake') {
            steps {
                echo 'Building devenv ... clang++ cmake '
		sh 'make image CXX=clang++ BUILDCHAIN=cmake'
            }
        }
    }
}