/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVD_TESTING_TEST_CALLER_H
#define UVD_TESTING_TEST_CALLER_H

#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 
#include "testing/serialized_test_result.h"

using namespace CppUnit;

template <class Fixture>
class UVTestCaller : public TestCaller<Fixture> {
public:
	//typedef TestCaller<Fixture>::TestMethod ATestMethod;
  typedef void (Fixture::*TestMethod)();

public:
  UVTestCaller( std::string name, TestMethod test ) :
	    TestCaller<Fixture>( name, test )
  {
  	init();
  }

  UVTestCaller(std::string name, TestMethod test, Fixture& fixture) :
	    TestCaller<Fixture>( name, test, fixture )
  {
  	init();
  }
    
  UVTestCaller(std::string name, TestMethod test, Fixture* fixture) :
	    TestCaller<Fixture>( name, test, fixture )
  {
  	init();
  }
  
  void init() {
   if(pipe(m_pipefd)) {
   //FIXME: do something better
   	throw std::exception();
   }       
  }
  
  ~UVTestCaller() {
  }

  virtual void run( TestResult *result ) {
	pid_t pid = 0;
	uint8_t buff[256];
	
	printf("About to fork()\n");
	fflush(stdout);
	fflush(stderr);	
	pid = fork();
	//Failed?
	CPPUNIT_ASSERT(pid >= 0);
	//Child?
	if (pid == 0) {
		try {
			CppUnit::TestResult *childResult = new CppUnit::TestResult();
			SerializedTestListener *l = new SerializedTestListener();
			childResult->addListener(l);
		
			close(m_pipefd[0]);
			//Assertions are thrown as exceptions, see Asserter::fail()
			//However, it seems that something catches them before they get this high
			printf("Child: running test\n");
			CppUnit::TestCaller<Fixture>::run(childResult);
			printf("CC*** Child: passed run, tests failed: %d\n", l->testsFailed());
		
			SerializedTestListener *deserialize(const void *buff, size_t bufferSize);
			size_t toWrite = l->serialize(&buff, sizeof(buff));
			uint8_t *writePos = buff;
			while (toWrite > 0) {
				int rc = write(m_pipefd[1], writePos, toWrite);
				if (rc < 0) {
					exit(1);
				}
				writePos += rc;
				toWrite -= rc;
			}
			//Generate EOF
			close(m_pipefd[1]);
			//Not sure how much these deletes really matter
			delete childResult;
			delete l;
			fflush(stdout);
			_exit(0);
		} catch(...) {
			_exit(1);
		}
	//Parent
	} else {
		//For now just try waiting on the child
		//This should cause all tests to pass since nothing failed
		pid_t waitrc = 0;
		int status = 0;
		
		waitrc = waitpid(pid, &status, 0);
		printf("\nChild done\n");
		CPPUNIT_ASSERT(waitrc == pid);
		if (WIFEXITED(status)) {
			printf("Child exited normally\n");
			printf("Exit code: %d\n", WEXITSTATUS(status));
			if (WEXITSTATUS(status)) {
				//TODO: throw something
			} else {
				/*
				Normal clean exit
				Try to de-serialized the piped data
				*/
				uint8_t *readPos = buff;
				unsigned int toRead = sizeof(buff);
				size_t nRead = 0;
				//Read as much as possible and then leave it to serializer object to see if it can decode it
				while (toRead > 0) {
					int rc = -1;
					struct timeval tv;
    				fd_set rfds;
    				
					FD_ZERO(&rfds);
					FD_SET(m_pipefd[0], &rfds);
					tv.tv_sec = 0;
					tv.tv_usec = 0;
					rc = select(m_pipefd[0] + 1, &rfds, NULL, NULL, &tv);
					if (rc != 1) {
						printf("No more data\n");
						break;
					}
				
					rc = read(m_pipefd[0], readPos, toRead);
					printf("read %d\n", rc);
					if (rc <= 0) {
						break;
					}
					readPos += rc;
					nRead += rc;
					toRead -= rc;
				}
				SerializedTestListener *ser = SerializedTestListener::deserialize(buff, nRead);
				if (!ser) {
					printf("deserialization failed\n");
				} else {
					ser->augmentResult(result, NULL);
				}
			}
		} else {
			printf("Child exited abnormally\n");
			
			if (WIFSIGNALED(status)) {
				printf("Child exited from signal %d\n", WTERMSIG(status));
				//TODO; throw something
			} else {
				//TODO; throw something
			}
		}
	}
  }
private:
	int m_pipefd[2];
};

#endif

