#include<DataX.h>
namespace nynn{namespace mm{
vector<xinetaddr_t>& loadconfig(const char*cfgpath,vector<xinetaddr_t> &cfg)
{
	char line[128];
	char hostname[32],hostaddr[32],port[32],*saveptr;
	xinetaddr_t iaddr;

	cfg.reserve(10);
	cfg.resize(0);

	fstream cfg_fin(cfgpath);

	if(cfg_fin.fail()){
		log_w("network configuration file is not found.");
		return cfg;
	}

	do{
		cfg_fin.getline(line,128);
		if(cfg_fin.fail()&&!cfg_fin.eof()){
			log_w("line width exceeds 128 bytes");
			return cfg;
		}

		//cin.get();
		//log_w("after line:%s",line);
		chop('#',line,line);
		//log_w("after chop(#,line):%s",line);
		ltrim(" \t",line,line);
		//log_w("after ltrim(\" \\t\",line):%s",line);
		rtrim(" \t",line,line);
		//log_w("after rtrim(\" \\t\",line):%s",line);
		if (strlen(line)!=0){
			strcpy(hostname,strtok_r(line," \t",&saveptr));
			strcpy(hostaddr,strtok_r(NULL," \t",&saveptr));
			strcpy(port,strtok_r(NULL," \t",&saveptr));

			iaddr._hostname=hostname;
			inet_pton(AF_INET,hostaddr,&iaddr._hostaddr);
			iaddr._port=htons((uint16_t)atoi(port));
			cfg.push_back(iaddr);
		}
	}while(!cfg_fin.eof());
	sort(cfg.begin(),cfg.end());
	return cfg;
}
}}
