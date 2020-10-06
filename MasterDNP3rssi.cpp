
#include <opendnp3/ConsoleLogger.h>

#include <opendnp3/DNP3Manager.h>

#include <opendnp3/channel/PrintingChannelListener.h>

#include <opendnp3/logging/LogLevels.h>

#include <opendnp3/master/DefaultMasterApplication.h>

#include <opendnp3/master/PrintingCommandResultCallback.h>

#include <opendnp3/master/PrintingSOEHandler.h>

#include <ctime>

#include <sys/time.h>

#include <climits>

#include<stdio.h>

#include <iostream>

#include <stdio.h>

#include <stdlib.h>

#include <string.h>


#include <unistd.h>

#include <time.h>

using namespace std;
using namespace opendnp3;

class TestSOEHandler: public ISOEHandler {
  virtual void BeginFragment(const ResponseInfo & info) {};
  virtual void EndFragment(const ResponseInfo & info) {};

  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < Binary >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < DoubleBitBinary >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < Analog >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < Counter >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < FrozenCounter >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < BinaryOutputStatus >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < AnalogOutputStatus >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < OctetString >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < TimeAndInterval >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < BinaryCommandEvent >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < Indexed < AnalogCommandEvent >> & values) {};
  virtual void Process(const HeaderInfo & info,
    const ICollection < DNPTime > & values) {};
};

FILE * pont_arq;
char n[2] = "\n";
char v[2] = ",";
double dt;
double dt1;
double t, t_atual;
int g;
double t0;

double t1;

double getCurrentRealTimer(void) {

  struct timeval t;

  gettimeofday( & t, 0);

  return t.tv_sec + t.tv_usec * 1.0e-6;

}

auto get1() {
  return [](const ICommandTaskResult & result) -> void {
    t = getCurrentRealTimer();
    dt = t - t1;
    t_atual = t - t0;
    dt = dt * 1000;
    std::cout << "Received command result w/ summary: " << TaskCompletionSpec::to_human_string(result.summary) <<
      std::endl;
    auto print = [](const CommandPointResult & res) {
      printf("\n %f", dt);
      fprintf(pont_arq, "%f", t_atual);
      fprintf(pont_arq, "%s", v);
      fprintf(pont_arq, "%f", dt);
      fprintf(pont_arq, "%s", v);
      
      std::cout << "Header: " << res.headerIndex << " Index: " << res.index <<
        " State: " << CommandPointStateSpec::to_human_string(res.state) <<
        " Status: " << CommandStatusSpec::to_human_string(res.status);
    };
    result.ForeachItem(print);
  };
}

int main(int argc, char * argv[]) {
  int i;
  int rssi1=0;
  int rssi2=0;
  // Specify what log levels to use. NORMAL is warning and above
  // You can add all the comms logging by uncommenting below
  const auto logLevels = levels::NORMAL | levels::ALL_APP_COMMS;

  // This is the main point of interaction with the stack
  DNP3Manager manager(1, ConsoleLogger::Create());

  // Connect via a TCPClient socket to a outstation
  auto channel = manager.AddTCPClient("tcpclient", logLevels, ChannelRetry::Default(), {
      IPEndpoint("192.168.26.122", 20000)
    },
    "0.0.0.0", PrintingChannelListener::Create());

  // The master config object for a master. The default are
  // useable, but understanding the options are important.
  MasterStackConfig stackConfig;

  // you can override application layer settings for the master here
  // in this example, we've change the application layer timeout to 2 seconds
  stackConfig.master.responseTimeout = TimeDuration::Seconds(2);
  stackConfig.master.disableUnsolOnStartup = true;

  // You can override the default link layer settings here
  // in this example we've changed the default link layer addressing
  stackConfig.link.LocalAddr = 1;
  stackConfig.link.RemoteAddr = 10;

  // Create a new master on a previously declared port, with a
  // name, log level, command acceptor, and config info. This
  // returns a thread-safe interface used for sending commands.
  auto master = channel -> AddMaster("master", // id for logging
    PrintingSOEHandler::Create(), // callback for data processing
    DefaultMasterApplication::Create(), // master application instance
    stackConfig // stack configuration
  );

  auto test_soe_handler = std::make_shared < TestSOEHandler > ();

  // do an integrity poll (Class 3/2/1/0) once per minute
  auto integrityScan = master -> AddClassScan(ClassField::AllClasses(), TimeDuration::Minutes(5), test_soe_handler);

  // do a Class 1 exception poll every 5 seconds
  auto exceptionScan = master -> AddClassScan(ClassField(ClassField::CLASS_1), TimeDuration::Seconds(380), test_soe_handler);

  // Enable the master. This will start communications.
  master -> Enable();

  bool channelCommsLoggingEnabled = true;
  bool masterCommsLoggingEnabled = true;

  while (true) {
    std::cout << "Enter a command" << std::endl;
    std::cout << "x - exits program" << std::endl;
    std::cout << "a - performs an ad-hoc range scan" << std::endl;
    std::cout << "i - integrity demand scan" << std::endl;
    std::cout << "e - exception demand scan" << std::endl;
    std::cout << "d - disable unsolicited" << std::endl;
    std::cout << "r - cold restart" << std::endl;
    std::cout << "c - send crob" << std::endl;
    std::cout << "t - toggle channel logging" << std::endl;
    std::cout << "u - toggle master logging" << std::endl;

    char cmd;
    std::cin >> cmd;
    switch (cmd) {
    case ('a'):
      master -> ScanRange(GroupVariationID(1, 2), 0, 3, test_soe_handler);
      break;
    case ('d'):
      master -> PerformFunction("disable unsol", FunctionCode::DISABLE_UNSOLICITED, {
        Header::AllObjects(60, 2),
        Header::AllObjects(60, 3),
        Header::AllObjects(60, 4)
      });
      break;
    case ('r'): {
      auto print = [](const RestartOperationResult & result) {
        if (result.summary == TaskCompletion::SUCCESS) {
          std::cout << "Success, Time: " << result.restartTime.ToString() << std::endl;
        } else {
          std::cout << "Failure: " << TaskCompletionSpec::to_string(result.summary) << std::endl;
        }
      };
      master -> Restart(RestartType::COLD, print);
      break;
    }
    case ('i'):
      integrityScan -> Demand();
      break;
    case ('e'):
      exceptionScan -> Demand();
      break;
    case ('x'):

      return 0;
    case ('c'): {
      t0 = getCurrentRealTimer();
      while (1) {
        pont_arq = fopen("Dadosdnp3.txt", "a");
		
        if (pont_arq == NULL) {
          printf("Erro na abertura do arquivo!");
          return 1;
        }
        t1 = getCurrentRealTimer();
        ControlRelayOutputBlock crob(OperationType::LATCH_ON);
        master -> SelectAndOperate(crob, 0, get1());
		system("python3 rssidnp3.py");
		usleep(4000000);
		FILE * pont_arq2;
		
		pont_arq2 = fopen("rssi.txt", "r");
		
		if (pont_arq2 == NULL) {
          printf("Erro na abertura do arquivo!");
          return 1;
        }
		
		if(!feof(pont_arq2) ) { 
			fscanf (fp,"%f",&rssi1);
			fscanf (fp,"%f",&rssi2);
		}
		
		  fclose(pont_arq2);
		
		fprintf(pont_arq, "%f",rssi1);
		fprintf(pont_arq, "%s", v);
		fprintf(pont_arq, "%f", rssi2);
        fclose(pont_arq);
      }
      break;
    }

    case ('t'): {
      channelCommsLoggingEnabled = !channelCommsLoggingEnabled;
      auto levels = channelCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
      channel -> SetLogFilters(levels);
      std::cout << "Channel logging set to: " << levels.get_value() << std::endl;
      break;
    }
    case ('u'): {
      masterCommsLoggingEnabled = !masterCommsLoggingEnabled;
      auto levels = masterCommsLoggingEnabled ? levels::ALL_COMMS : levels::NORMAL;
      master -> SetLogFilters(levels);
      std::cout << "Master logging set to: " << levels.get_value() << std::endl;
      break;
    }
    default:
      std::cout << "Unknown action: " << cmd << std::endl;
      break;
    }
  }

  return 0;
}