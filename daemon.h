#include <signal.h>
#include <stdlib.h>
#include <string>

using namespace std;

string atexit_pidfile;
void delpid(){
	unlink(atexit_pidfile.c_str());
}

class Daemon{
public:
	Daemon(string pidfile, string in, string out, string err):pidfile(pidfile),in(in),out(out),err(err){
		atexit_pidfile = pidfile;
	}

	void daemonize(){
		pid_t pid = fork();
		if(pid > 0){
			exit(0);
		}
		else if(pid < 0){
			fprintf(stderr, "Unable to fork");
			exit(1);
		}

		chdir("/");
		setsid();
		umask(0);

		// UNIX double-fork magic
		pid = fork();
		if(pid > 0){
			exit(0);
		}
		else if(pid < 0){
			fprintf(stderr, "Unable to fork #2");
			exit(1);
		}

		fflush(stdout);
		fflush(stderr);

		freopen(in.c_str(),"r",stdin);
		freopen(out.c_str(),"w",stdout);
		freopen(err.c_str(),"w",stderr);

		pid = getpid();
		FILE *pidfd = fopen(pidfile.c_str(), "w+");
		if(pidfd){
			fprintf(pidfd, "%d", pid);
			fclose(pidfd);
		}

		atexit(delpid);
	}

	void start(){
		int pid=0;
		FILE *pidfd = fopen(pidfile.c_str(), "r");
		if(pidfd){
			fscanf(pidfd, "%d", &pid);
			fclose(pidfd);
		}

		if(pid){
			fprintf(stderr,"pidfile %s already exists. Daemon already running?\n",pidfile.c_str());
			exit(1);
		}

		daemonize();
		run();
	}

	void stop(){
		int pid=0;
		FILE *pidfd = fopen(pidfile.c_str(), "r");
		if(pidfd){
			fscanf(pidfd, "%d", &pid);
			fclose(pidfd);
		}

		if(!pid){
			fprintf(stderr,"pidfile %s does not exist.  Daemon not running?\n",pidfile.c_str());
			return;
		}

		int result = kill(pid, SIGTERM);
		if(result < 0){
			fprintf(stderr,"Unable to kill process\n");
			exit(1);
		}
		else{
			unlink(pidfile.c_str());
		}
	}

	void restart(){
		stop();
		start();
	}

	virtual void run()=0;

protected:
	string pidfile;
	string in,out,err;
};

