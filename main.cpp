#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <streambuf>
#include <string.h>
using namespace std;

/// Codes used for both encryption and decryption
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
	if(argc<3){ /// Prints usage and exits if ther is not enough program arguments
		clog<<"[DOSYA ADI] [SIFRE(en fazla 18 harf)] [RASTGELE KARAKTER SIKLIGI(varsayilan=8)] [TEKRAR SAYISI(varsayilan=9)]"<<endl;
		return 1;
	}

	/// Variable definitions
	string fname=argv[1];
	char pass[19]={0},xorkey[16],shkey[16],xb[16],sb[16],buf,pbuf[37];
	unsigned short rbr,itc,cnt=0;
	vector<char>::iterator contit;
	strncpy(pass,argv[2],18);

	fstream kf(fname+".key",ios::in|ios::binary); /// Try to open .key file
	/// .key file exists = decryption
	/// .key file doesn't exist = encryption
	if(kf.good()){

		kf.read(pbuf,36); /// Read the .key file
		kf.close();

		for(unsigned short i=0;i<36;i++){ /// Decrypt .key file
            pbuf[i]^=pass[17-(i/2)]*pass[i%2?i/2:17-(i/2)];
            pbuf[i]-=pass[i/2]*pass[!(i%2)?i/2:17-(i/2)];
        }

        /// Read keys, random byte frequency and iteration count
        strncpy(xorkey,pbuf,16);
        strncpy(shkey,pbuf+16,16);
        strncpy(reinterpret_cast<char*>(&rbr),pbuf+32,2);
        strncpy(reinterpret_cast<char*>(&itc),pbuf+34,2);

		kf.open(fname+".enc",ios::in|ios::binary); /// Open .enc file
		if(!kf.good()){ /// If .enc file doesn't exist, print error message and exit
			clog<<"Hata: Dosya bulunamadi."<<endl;
			return 1;
		}
		vector<char> cont(istreambuf_iterator<char>(kf),(istreambuf_iterator<char>())); /// Copy .enc file's content to vector
		kf.close();

		for(unsigned short i=0;i<16;i++){ /// Save keys
			xb[i]=xorkey[i];
			sb[i]=shkey[i];
		}

		for(unsigned short j=0;j<itc;j++){ /// Start decrypting
			cout<<"Dosya cozuluyor ("<<j+1<<'/'<<itc<<')'<<endl;

			cnt=0;
			XOR(itc-j);

			cnt=0;
			SH(itc-j,-=);
			for(unsigned short i=0;i<16;i++){ /// Reset keys
				xorkey[i]=xb[i];
				shkey[i]=sb[i];
			}
		}

		/// Remove random bytes and print to file
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

		/// Exit the program
		cout<<"Dosya cozuldu."<<endl;
		return 0;
	}else{
		kf.close();

		{   /// Create keys
			random_device e;
			uniform_int_distribution<int> d(static_cast<int>(std::numeric_limits<char>::min()),static_cast<int>(std::numeric_limits<char>::max()));
			for(auto& i:xorkey) i=d(e);
			for(auto& i:shkey) i=d(e);
		}

		/// Set random byte frequency and iteration count
		rbr=argc>3?atoi(argv[3]):8;
		itc=argc>4?atoi(argv[4]):9;

		/// Copy keys, random byte frequency and iteration count into pbuf
        strcpy(pbuf,xorkey);
        strcpy(pbuf+16,shkey);
        strcpy(pbuf+32,reinterpret_cast<char*>(&rbr));
        strcpy(pbuf+34,reinterpret_cast<char*>(&itc));

        for(unsigned short i=0;i<36;i++){ /// Encrypt pbuf
            pbuf[i]+=pass[i/2]*pass[!(i%2)?i/2:17-(i/2)];
            pbuf[i]^=pass[17-(i/2)]*pass[i%2?i/2:17-(i/2)];
        }

        /// Write pbuf to .key file
        ofstream pf(fname+".key",ios::out|ios::trunc|ios::binary);
        pf.write(pbuf,36);
        pf.close();

		kf.open(fname,ios::in|ios::binary); /// Open file
		if(!kf.good()){ /// If file doesn't exist, print error message, remove .key file and exit
			clog<<"Hata: Dosya bulunamadi."<<endl;
			remove((fname+".key").c_str());
			return 1;
		}

		vector<char> cont;
		for(unsigned short i=0;i<16;i++){ /// Save keys
			xb[i]=xorkey[i];
			sb[i]=shkey[i];
		}

		/// Read file and place random bytes
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

		for(unsigned short j=0;j<itc;j++){ /// Start encryption
			cout<<"Dosya sifreleniyor ("<<j+1<<'/'<<itc<<')'<<endl;

			cnt=0;
			SH(j+1,+=);

			cnt=0;
			XOR(j+1);
			for(unsigned short i=0;i<16;i++){ /// Reset keys
				xorkey[i]=xb[i];
				shkey[i]=sb[i];
			}
		}

		/// Create .enc file and write encrypted data to file
		kf.open(fname+".enc",ios::out|ios::binary);
		kf<<string(cont.begin(),cont.end());
		kf.close();

		/// Exit the program
		cout<<"Dosya sifrelendi."<<endl;
		return 0;
	}
}
