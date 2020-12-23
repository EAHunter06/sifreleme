#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <streambuf>
#include <string.h>
using namespace std;
#define XOR(val) for(contit=cont.begin();contit!=cont.end();contit++){\
	*contit^=xorkey[cnt++];\
	if(cnt==16){\
		cnt=0;\
		for(auto& i:xorkey) i*=i*(val);\
	}\
}(void*)(0)
#define SH(val,op) for(contit=cont.begin();contit!=cont.end();contit++){\
	*contit op shkey[cnt++];\
	if(cnt==16){\
		cnt=0;\
		for(auto&i:shkey) i*=i*(val);\
	}\
}(void*)(0)
int main(int argc,char **argv){
	if(argc<3){
		clog<<"[DOSYA ADI] [SIFRE(en fazla 18 harf)] [RASTGELE KARAKTER SIKLIGI(varsayilan=8)] [TEKRAR SAYISI(varsayilan=9)]"<<endl;
		return 1;
	}

	string fname=argv[1];
	char pass[19]={0},xorkey[16],shkey[16],xb[16],sb[16],buf,pbuf[37];
	unsigned short rbr,itc,cnt=0;
	vector<char>::iterator contit;
	strncpy(pass,argv[2],18);

	fstream kf(fname+".key",ios::in|ios::binary);
	if(kf.good()){
		//Sifre al
		kf.read(pbuf,36);
		kf.close();

		for(unsigned short i=0;i<36;i++){
            pbuf[i]^=pass[17-(i/2)]*pass[i%2?i/2:17-(i/2)];
            pbuf[i]-=pass[i/2]*pass[!(i%2)?i/2:17-(i/2)];
        }

        strncpy(xorkey,pbuf,16);
        strncpy(shkey,pbuf+16,16);
        strncpy(reinterpret_cast<char*>(&rbr),pbuf+32,2);
        strncpy(reinterpret_cast<char*>(&itc),pbuf+34,2);

		//Dosya ac
		kf.open(fname+".enc",ios::in|ios::binary);
		if(!kf.good()){
			clog<<"Hata: Dosya bulunamadi."<<endl;
			return 1;
		}
		vector<char> cont(istreambuf_iterator<char>(kf),(istreambuf_iterator<char>()));
		kf.close();

		for(unsigned short i=0;i<16;i++){
			xb[i]=xorkey[i];
			sb[i]=shkey[i];
		}

		for(unsigned short j=0;j<itc;j++){
			cout<<"Dosya cozuluyor ("<<j+1<<'/'<<itc<<')'<<endl;
			//Xor
			cnt=0;
			XOR(itc-j);
			//Kaydir
			cnt=0;
			SH(itc-j,-=);
			for(unsigned short i=0;i<16;i++){
				xorkey[i]=xb[i];
				shkey[i]=sb[i];
			}
		}

		//Rastgeleleri sil ve yaz
		cout<<"Rastgele karakterler siliniyor"<<endl;
		kf.open(fname,ios::out|ios::binary);
		cnt=0;
		for(contit=cont.begin();contit!=cont.end();contit++){
			if(cnt++==rbr) cnt=0;
			else{
				buf=*contit;
				kf.write(&buf,1);
			}
		}
		kf.close();
		cout<<"Dosya cozuldu."<<endl;
		return 0;
	}else{
		//Sifre olustur
		kf.close();

		{
			random_device e;
			uniform_int_distribution<int> d(static_cast<int>(std::numeric_limits<char>::min()),static_cast<int>(std::numeric_limits<char>::max()));
			for(auto& i:xorkey) i=d(e);
			for(auto& i:shkey) i=d(e);
		}

		rbr=argc>3?atoi(argv[3]):8;
		itc=argc>4?atoi(argv[4]):9;

        strcpy(pbuf,xorkey);
        strcpy(pbuf+16,shkey);
        strcpy(pbuf+32,reinterpret_cast<char*>(&rbr));
        strcpy(pbuf+34,reinterpret_cast<char*>(&itc));

        for(unsigned short i=0;i<36;i++){
            pbuf[i]+=pass[i/2]*pass[!(i%2)?i/2:17-(i/2)];
            pbuf[i]^=pass[17-(i/2)]*pass[i%2?i/2:17-(i/2)];
        }

        ofstream pf(fname+".key",ios::out|ios::trunc|ios::binary);
        pf.write(pbuf,36);
        pf.close();

		kf.open(fname,ios::in|ios::binary);
		if(!kf.good()){
			clog<<"Hata: Dosya bulunamadi."<<endl;
			remove((fname+".key").c_str());
			return 1;
		}

		vector<char> cont;
		for(unsigned short i=0;i<16;i++){
			xb[i]=xorkey[i];
			sb[i]=shkey[i];
		}

		//Karakter serp
		cout<<"Rastgele karakterler yerlestiriliyor"<<endl;
		{
			random_device e;
			mt19937 r(e());
			while(kf>>noskipws>>buf){
				cont.push_back(buf);
				if(++cnt==rbr){
					cnt=0;
					cont.push_back(static_cast<char>(r()));
				}
			}
		}
		kf.close();

		for(unsigned short j=0;j<itc;j++){
			cout<<"Dosya sifreleniyor ("<<j+1<<'/'<<itc<<')'<<endl;
			//Kaydir
			cnt=0;
			SH(j+1,+=);
			//Xor
			cnt=0;
			XOR(j+1);
			for(unsigned short i=0;i<16;i++){
				xorkey[i]=xb[i];
				shkey[i]=sb[i];
			}
		}

		//Yaz
		kf.open(fname+".enc",ios::out|ios::binary);
		kf<<string(cont.begin(),cont.end());
		kf.close();
		cout<<"Dosya sifrelendi."<<endl;
		return 0;
	}
}
