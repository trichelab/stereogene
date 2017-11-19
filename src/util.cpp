/*
 * util.cpp
 *
 *  Created on: 17.02.2013
 *      Author: 1
 */

#include "track_util.h"
#include <time.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/file.h>
//#include <dir.h>
const char* version="2.12";

int debugFg=0;
//int debugFg=DEBUG_LOG|DEBUG_PRINT;

const char *debS=0;

Chromosome *chrom_list;       // list of chromosomes
Chromosome *curChrom=chrom_list;
int  binSize=100;   // frame size fo profile
bool  NAFlag=0;

long long GenomeLength=0;      // TOTAL LENGTH OF THE GENOME
int n_chrom;
char trackName[4096];       // current track name
char *chromFile=0;
char *confFile=0;		// confounder file name
char *cfgFile=0;		// config file name
char *profPath=0;
char *trackPath=0;

char *trackFil=0;		// Track file
char *aliaseFil=0;
char *logFileName=(char*)"./stereogene.log";
char *outFile=0;
char *profile1=0;		// first profile file file name
char *profile2=0;		// second profile file file name
char *resPath=0;
char *statFileName=(char*)"./statistics";
char *paramsFileName=(char*)"./params";
char *inputProfiles=0;
char *outTrackFile=0; // Filename for write out track
char *idSuff=(char*)"";

bool  verbose=0;
bool  silent=0;				// inhibit stdout
bool  syntax=1;				// Strong syntax control

bool  writeDistr=1;
bool  writeDistCorr=1;		    // write BroadPeak
int   crossWidth=10000;
bool  outSpectr=0;
bool  outChrom=0;
int   outRes=XML|TAB;
int   inpThreshold=0;		// Testing of binarized input data, % of max
bool  writePDF=true;
int   complFg=IGNORE_STRAND;
int   lcFlag=CENTER;
int   profileLength;			// size of the profile array

char *pcorProfile=0;    // partial correlation profile file name
int  binBufSize=30000000;


int 	kernelType=KERN_NORM;
char* 	customKern=0;
double 	noiseLevel=0.2;
int 	wSize=100000;        // size of widow (nucleotides)
int 	wStep=0;             // window step   (nucleotides)
int 	flankSize=0;
double 	kernelSigma=1000.;    	// kernel width (nucleotides)
double 	kernelShift=0;      	    // Kernel mean (for Gauss) or Kernel start for exponent
int 	intervFg0;
double 	scaleFactor=0.2;
bool 	outLC=0;
int 	LCScale=LOG_SCALE;
//int 	LCScale=LIN_SCALE;
double LlcFDR=0;		// treshold on FDR when write Local Correlation track
double RlcFDR=0.5;		// treshold on FDR when write Local Correlation track

bool 	outPrjBGr=true;

int 	wProfStep=0;          	// window step   (profile scale)
int 	wProfSize=0;          	// size of widow (profile scale)
int 	LFlankProfSize=0;         // size of flank (profile scale)
int 	RFlankProfSize=0;         // size of flank (profile scale)
int 	profWithFlanksLength=0; 	// size of profWindow array (including random flanks)
double 	kernelProfSigma=1000;     // kernel width ((profile scale)
double 	kernelProfShift=0;
double 	kernelNS=0;			// Correction for non-specifisity
Track 	*track1=0, *track2=0, *projTrack=0;
Kernel 	*kern=0;
double 	maxNA0=95;
double 	maxZero0=95;
double 	maxNA=100;
double 	maxZero=100;
int 	nShuffle=10000;
Model 	*model;

int 	threshold=0;

FILE 	*logFile=0;
bool 	doAutoCorr=0;

int 	corrScale=10;
double 	prod11=0,prod12=0,prod22=0, eprod1,eprod2;
int 	nprod=0;
XYCorrelation XYfgCorrelation;		    // array for correlation picture
XYCorrelation XYbgcorrelation;			// array for correlation picture
Fourier LCorrelation;

double 	totCorr=0, BgTotal=0;
unsigned long id;
bool 	RScriptFg=0;
int 	bpType=BP_SIGNAL;
int 	cage=0;
bool 	clearProfile=false;
int 	scoreType=AV_SCORE;
AliasTable alTable;
FileListEntry files[256];
int   	nfiles;
bool LCExists=false;

double 	BgAvCorr=0;
double 	FgAvCorr=0;
int  	pgLevel=2;
float total=0;						// total count over the track



unsigned int hashx(unsigned int h,unsigned int x);
unsigned int hashx(unsigned int h,char c);
unsigned int hashx(unsigned int h,int x);
unsigned int hashx(unsigned int h,long x);
unsigned int hashx(unsigned int h,const char *s);
unsigned int hashx(unsigned int h,float c);
unsigned int hashx(unsigned int h,double c);

//====================================================================================
ScoredRange::ScoredRange(){
	chr=0; chrom=0;	beg=end=0;	score=0;
}
//==================================== print a range to a BED GRAPH
void ScoredRange::printBGraph(FILE *f){
	if(score!=NA){
		fprintf(f,"%s\t%li\t%li\t",chrom,beg, end);
		double xx=abs(score);
		if(xx < 0.000001) 	fprintf(f,"0.0\n");
		else if(xx < 0.0001) 	fprintf(f,"%.6f\n",score);
		else if(xx < 0.001) fprintf(f,"%.5f\n",score);
		else if(xx < 0.01)  fprintf(f,"%.4f\n",score);
		else if(xx < 0.1)   fprintf(f,"%.3f\n",score);
		else                fprintf(f,"%.2f\n",score);
	}
//	else
//		fprintf(f,"#%s\t%li\t%li\t?\n",chrom,beg, end);
}


//====================================================================================
char *AliasTable::convert(char*oldName, char *newName){
	char b0[1024],b1[1024];
	strcpy(b0,oldName);

	for(int i=0; i<nAls; i++){
		while(1){
			char* sx=strstr(b0,als[i].oldName);
			if(sx!=0){
				int k=sx-b0;
				char *s0=b0,*s1=b1;
				memcpy(s1,s0,k); s0+=k; s1+=k;
				memcpy(s1,als[i].newName,als[i].lnew);
				s0+=als[i].lold; s1+=als[i].lnew;
				strcpy(s1,s0);
				strcpy(b0,b1);
			}
			else break;
		}
	}
	return strcpy(newName, b0);
}

void AliasTable::readTable(const char* fname){
	FILE *f=gopen(fname,"rt");
	if(f==0) return;
	nAls=0;
	int capacity=100;
	getMem(als,capacity, "AliaseTable::readTable");
	char b[1024],*s;

	for(;(s=fgets(b,sizeof(b),f))!=0;){
		strtok(b,"\r\n");
		s=skipSpace(b);
		if(*s=='#') continue;

		char *s1=strtok(s,"=");
		char *s2=strtok(0," \t");
		s1=strtok(s1," \t");
		if(s1==0 || strlen(s1)==0) continue;
		if(s2!=0) s2=skipSpace(s2);
		else s2=(char*)"";
		als[nAls].oldName=strdup(s1);
		als[nAls].newName=strdup(s2);
		als[nAls].lnew=strlen(s2);
		als[nAls].lold=strlen(s1);
		nAls++;
		if(nAls >= capacity){
			capacity+=100;
			als=(Alias*)realloc(als,capacity*sizeof(Alias));
		}
	}
	fclose(f);
}

//====================================================================================
//===============================================  Chromosomes
Chromosome::Chromosome(char *chr, long l, int bb){
    chrom=chr;		//Chromosome name
    length=l;		//Chromosome length
    base=bb;		//Start position in the binary profile
    av1=av2=corr=lCorr=count=0;
    densCount=0;
    distDens=0;
}

void Chromosome::clear(){
    av1=av2=corr=lCorr=count=0; densCount=0;
    if(profWithFlanksLength) {
    	getMem0(distDens,profWithFlanksLength,  "Chromosome::clear #1");
    	zeroMem(distDens,profWithFlanksLength);
    }
}

void clearChromosomes(){
	for(int i=0; i<n_chrom; i++){
		chrom_list[i].clear();
	}
}


int CHROM_BUFF=300;

int readChromSizes(char *fname){

	if(fname==0) errorExit("Chromosome file undefined");
	FILE *f=xopen(fname,"rt");
	if(f==0)return 0;
	char buff[2048];
	n_chrom=0;
	int max_chrom=CHROM_BUFF;
	chrom_list=(Chromosome *)malloc(max_chrom*sizeof(Chromosome));
	char *s1,*s2;

	for(char *s; (s=fgets(buff,2048,f))!=0;){
		s=trim(buff);
		if(*s==0 || *s=='#') 	continue;

		if(n_chrom>=max_chrom) {
			max_chrom+=CHROM_BUFF;
			chrom_list=(Chromosome *)realloc(chrom_list,max_chrom*sizeof(Chromosome));
		}
	    if((s1=strtok(s," \t\r\n"))==NULL) continue;
	    if((s2=strtok(0," \t\r\n"))==NULL) continue;

	    chrom_list[n_chrom]=Chromosome(strdup(s1), atol(s2), profileLength);

		int filLen=(chrom_list[n_chrom].length+binSize-1)/binSize;
		profileLength+=filLen;

		GenomeLength+=chrom_list[n_chrom].length;
	    n_chrom++;

	}
	curChrom=chrom_list;
	return 1;
};

Chromosome* findChrom(char *ch){
	if(strcmp(curChrom->chrom, ch) ==0) return curChrom;
	for(int i=0; i<n_chrom; i++){
		curChrom=chrom_list+i;
		if(strcmp(curChrom->chrom, ch) ==0) return curChrom;
	}
	fprintf(stderr,"Chromosome %s not found\n",ch);
	return 0;
}
//========================================================================================
long pos2filePos(char*chrom,long pos){
	Chromosome *ch=findChrom(chrom);
	if(ch==0) return 0;
	curChrom=ch;
	long p=pos/binSize+curChrom->base;
	return p;
}

//========================================================================================
Chromosome *getChromByPos(int pos){
	Chromosome *ch0=chrom_list;
	for(int i=0; i<n_chrom; i++){
		if(pos < chrom_list[i].base) return ch0;
		ch0=chrom_list+i;
	}
	return ch0;
}

//========================================================================================
void filePos2Pos(int pos, ScoredRange *gr, int length){
	Chromosome *ch0=getChromByPos(pos);

	if(ch0==0) return;
	pos-=ch0->base;
	long p1=binSize*long(pos);
	gr->chr=ch0;
	gr->chrom=ch0->chrom;
	gr->end=(gr->beg=p1)+length;
	if(gr->end >= ch0->length) gr->end = ch0->length-1;
	return;
}

//========================================================================================
int inputErr;		// flag: if input track has errors
int inputErrLine;	// Error line in the input
char curFname[4048];	// current input file

//========================================================================================
Chromosome *checkRange(ScoredRange *gr){
	Chromosome* chr=findChrom(gr->chrom);
	if(chr==0) return 0;

	if(gr->beg < 0){
		if(inputErr == 0){ inputErr=1;
			writeLogErr(
					"File <%s> line #%d: incorrect segment start: chrom=%s  beg=%ld.  Ignored\n",curFname, inputErrLine,gr->chrom, gr->beg);
		}
		return 0;
	}
	if(gr->end  >= chr->length){
		if(inputErr == 0){ inputErr=1;
			writeLogErr(
					"File <%s> line #%d: incorrect segment end: chrom=%s  end=%ld.  Ignored\n",curFname, inputErrLine,gr->chrom, gr->end);
		}
		return 0;
	}
	if(gr->end < gr->beg) return 0;
	return chr;
}

//======================================================================================
//============================    String  Parsing ======================================
//======================================================================================
char *skipSpace(char *s)  {while(*s!=0 &&  isspace(*s)) s++; return s;}
char *skipNoSpace(char *s){while(*s!=0 && !isspace(*s)) s++; return s;}
int EmptyString(const char*buff){
	for(const char*s=buff; *s!=0; s++)
		if(!isspace(*s)) return 0;
	return 1;
}

//==============================================================================
bool isUInt(const char *s){
	for(;*s;s++) {
		if(!isdigit(*s)) return false;
	}
	return true;
}
//==============================================================================
bool isInt(const char *s){
	char *ss=skipSpace((char*)s);
	if(*ss=='-' || *ss=='+') ss++;
	return isUInt(ss);
}
//==============================================================================
bool isDouble(const char *s){
	char b[256]; strcpy(b,s);
	char *s0=strtok(b,"eE");
	char *s1=strtok(0,"");
	if(s1!=0 && strlen(s1)!=0 && !isInt(s1)) return false;
	s0=strtok(s0,"."); s1=strtok(0,".");
	if(!isInt(s0)) return false;
	if(s1!=0 && strlen(s1)!=0 && !isUInt(s1)) return false;
	return true;
}

//=================================== extract attribute value by attr name
char * getAttr(char *s0, const char *name, char *buf){
	char *s=s0;
	int ll=strlen(name);
	char *ss=buf;
	while(*s!=0){
		s=strchr(s,*name);
		if(s==0) return 0;
		if(strncmp(s,name,ll)!=0) { s++; continue;}
		s=skipSpace(s+strlen(name));
		if(*s==0 || *s!='=') continue;
	    s=skipSpace(s+1);
		if(*s!='\"'){while(*s!=0 && !isspace(*s)) *ss++=*s++;}
		else{   s++; while(*s!=0 && *s != '\"'  ) *ss++=*s++;}
		*ss=0; return buf;
	}
	return 0;
}

//=================================== convert string to upper case
char *strtoupper(char*s){
	for(char *ss=s;*ss;ss++) *ss=toupper(*ss);
	return s;
}

// get major version number (1.64.2 -> 1.64)
char * getMajorVer(const char *ver, char *buf){
	char b[1024];
	strcpy(b,ver);
	strcpy(buf,strtok(b,"."));
	strcat(buf,".");
	strcat(buf,strtok(0,"."));
	return buf;
}
//===================== check if given string contains given key: 0 -- contains; 1 -- does not
int keyCmp(const char *str, const char *key){
	for(;;str++, key++){
		if(*str==0){
			if(*key==0) return 0;
			else return 1;
		}
		if(*key==0) return -1;
		if(toupper(*str) != toupper(*key)) return 1;
	}
	return strncmp(str,key,strlen(key));
}
//========================= trim given string
char *trim(char *s){
	if(s==0) return 0;
	s=skipSpace(s);
	for(int i=strlen(s)-1; i>=0; i--){
		if(isspace(s[i])) s[i]=0;
		else break;
	}
	if(*s=='\"') s++;
	char *ss=strrchr(s,'\"'); if(ss) *ss=0;
	return s;
}


//========== Convert kernel type to a string
char kernType[1024];
const char*getKernelType(){
	const char *type;
	if(kernelType==KERN_NORM	 ) type="N";
	else if(kernelType==KERN_LEFT_EXP ) type="L";
	else if(kernelType==KERN_RIGHT_EXP) type="R";
	else if(kernelType==KERN_CUSTOM) type=customKern;
	else return "X";
	char b[80];
	if(kernelShift >0){
		sprintf(b,"%s_%.1fR",type,kernelShift/1000);
		return strcpy(kernType,b);
	}
	else if(kernelShift <0){
		sprintf(b,"%s_%.1fL",type,-kernelShift/1000);
		return strcpy(kernType,b);
	}
	else return type;
}


//============================================================
//=======================    Logging    ======================
//============================================================
const char *errStatus=0;
void clearLog(){
	if(logFileName) fclose(gopen(logFileName,"wt"));
}


FILE *openLog(){
	if(logFileName) {
		char b[2048];
		if(*logFileName=='$'){
			if(outFile && *outFile){
				sprintf(b,"%s%s",outFile,logFileName+1);
			}
			else
				sprintf(b,"log%s",logFileName+1);
		}
		else strcpy(b,logFileName);
		FILE *f=gopen(b,"at");
		if(f!=0) return f;
		else{
			fprintf(stderr, "Error in opening log file%s Error code=%i\n", logFileName, errno);
		}
	}
	return 0;
}

void writeLog(const char *format, va_list args){
	FILE *f=openLog();
	if(f) {
		flockFile(f);
		if(!debugFg) fprintf(f,"#%08lx-> ",id);
		vfprintf(f,format,args);
		funlockFile(f);
		fclose(f);
	}
}

void writeLog(const char *format, ...){
	va_list args;
	va_start(args, format);
	writeLog(format, args);
	va_end(args);
}
void writeLogErr(const char *format, ...){
	char b[2048];
	va_list args;
	va_start(args, format);
	vsprintf(b, format,args);
	va_end(args);
	writeLog(b);
	fprintf(stderr,"%s",b);
}

//============================================================
//=======================    Verbose    ======================
//============================================================
void verb_(const char *format, va_list args){	//======== write if verbose =1
	if(verbose){
		vprintf(format,args);
	}
}
void verb(const char *format, ...){
	va_list args;
	va_start(args, format);
	verb_(format, args);
	va_end(args);
	fflush(stdout);
}
void xverb_(const char *format, va_list args){   //======== write if silent =0
	if(!silent){
		vprintf(format,args);
	}
}
void xverb(const char *format, ...){
	va_list args;
	va_start(args, format);
	xverb_(format, args);
	va_end(args);
	fflush(stdout);
}

//============================================================
//=======================    Errors    =======================
//============================================================
void errorExit(const char *format, va_list args){
    fflush(stdout);
	if (format != NULL) {
		char b[1024];
		vsprintf(b, format, args);
	    fprintf(stderr, "%s", b);
	    if(errStatus) fprintf(stderr, " %s\n", errStatus);
	    else fprintf(stderr, "\n");
	    FILE *f=openLog();
		if(f) {
			flockFile(f);
			fprintf(f,"#%08lx-> %s",id,b);
			if(errStatus) fprintf(f, "%s\n", errStatus);
			else fprintf(f, "\n");
			funlockFile(f);
			fclose(f);
		}
	}
	exit(-1);
}
void errorExit(const char *format, ...){
	va_list args;
	va_start(args, format);
	errorExit(format, args);
	va_end(args);
}


//============================================================
//=======================    Debug     =======================
//============================================================
Timer debTimer;
FILE *debLogFile=0;
void clearDeb(){
	if((debugFg&DEBUG_LOG)!=0){
		if(debLogFile==0) debLogFile=fopen("deb_log","wt");
		fclose(debLogFile); debLogFile=0;
	}
}
//===========================================
void _deb_(bool t, const char *format, va_list args){
	char b[2048];
	vsprintf(b, format, args);
	if((debugFg&DEBUG_PRINT)!=0){
		printf("%s",b);
		if(t) printf(" %s",debTimer.getTime());
		printf("\n");
	}
	if((debugFg&DEBUG_LOG)!=0){
		if(debLogFile==0) debLogFile=fopen("deb_log","a+t");
		fprintf(debLogFile, "%s",b);
		if(t) fprintf(debLogFile, " %s",debTimer.getTime());
		fprintf(debLogFile, "\n");
		fclose(debLogFile); debLogFile=0;
	}
	if(t) debTimer.reset();
}
//===========================================

void deb(int num, bool t, char e){
	if((debugFg&DEBUG_PRINT)!=0){
		if(debS) printf("%s",debS);
		printf(" #%i",num);
		if(t) printf(" %s",debTimer.getTime());
		printf("%c",e);
	}
	if((debugFg&DEBUG_LOG)!=0){
		if(debLogFile==0) debLogFile=fopen("deb_log","a+t");
		if(debS) fprintf(debLogFile, "%s",debS);
		fprintf(debLogFile, " #%i",num);
		if(t) fprintf(debLogFile, " %s",debTimer.getTime());
		fprintf(debLogFile, "%c",e);
		fclose(debLogFile); debLogFile=0;
	}
	if(t) debTimer.reset();
}

void deb(int num){
	deb(num, false,'\n');
}

void deb(const char *format, ...){
	va_list args;
	va_start(args, format);
	_deb_(false, format, args);
	va_end(args);
}

void deb(int num, const char *format, ...){
	if(debugFg==0) return;
	deb(num,false,' ');
	va_list args;
	va_start(args, format);
	_deb_(false, format, args);
	va_end(args);
}
void debt(int num){
	deb(num,true,'\n');
}
void debt(const char *format, ...){
	va_list args;
	va_start(args, format);
	_deb_(true, format, args);
	va_end(args);
}

void debt(int num, const char *format, ...){
	if(debugFg==0) return;
	deb(num,false,' ');
	va_list args;
	va_start(args, format);
	_deb_(true, format, args);
	va_end(args);
}
void debt(){
	debTimer.reset();
}
//============================================================
//=======================      Timer   =======================
//============================================================
char *Timer::getTime(){
	long dt=getTimer();
	int ms=(int)(dt%1000);
	int s=(int)(dt/1000);
	sprintf(bb,"%i.%is",s,ms);
	return bb;
}

Timer::Timer(){
	reset();
}
void Timer::reset(){
	start=mtime();
}

long Timer::getTimer(){
	long curTime=mtime();
	return curTime-start;
}

long mtime()
{
  struct timeval t;

  gettimeofday(&t, NULL);
  long mt = (long)t.tv_sec * 1000 + t.tv_usec / 1000;
  return mt;
}

char timerBufferQQ[256];
char *dateTime(){
	time_t lt=time(NULL);
	tm *t=localtime(&lt);
	sprintf(timerBufferQQ,"%02i.%02i.%02i %02i:%02i:%02i",t->tm_mday, t->tm_mon+1, t->tm_year%100,
			t->tm_hour, t->tm_min, t->tm_sec);
	return timerBufferQQ;
}

//============================================================
//====================   Files and Paths    ==================
//============================================================
char* parseTilda(char *b, const char*fname){
	if(*fname=='~'){
		char *z=getenv("HOME");
		if(z==0) z=getenv("HOMEPATH");
		if(z!=0) {
			strcpy(b,correctFname(z));
			if(b[strlen(b)-1] != '/') strcat(b,"/");
			fname+=2;}
	}
	else *b=0;
	return strcat(b,fname);
}

//================ open file with control
FILE *xopen(const char* fname, const char *t){
	if(fname==0) errorExit("can\'t open file <null>");
	FILE* f=gopen(fname,t);
	if(f==0){
		char b[2048];
		errorExit("can\'t open file <%s> (<%s>)",fname, parseTilda(b,fname));
	}
	return f;
}

FILE *gopen(const char*fname, const char* type){		// open file with parsing ~
	char b[2048];
	return fopen(parseTilda(b,fname),type);
}
// remove fucked backslash
char *correctFname(char* s){
	char *ss;
	for(ss=s;*ss;ss++) if(*ss=='\\') *ss='/';
	return s;
}
//================= create filename using path and name
char* makeFileName(char *b, const char *path, const char*fname){
	if(path==0) return strcpy(b,fname);
	if(*fname=='/' || *fname=='~') return strcpy(b,fname);
	if(path && path[strlen(path)-1]=='/') sprintf(b,"%s%s",path,fname);
	else								  sprintf(b,"%s/%s",path,fname);
	return b;
}
//================= create filename using path and name
char *makeFileName(char *b, const char *path, const char*fname, const char*ext){
	makeFileName(b,path,fname);
	char *ss=strrchr(b,'/'); if(ss==0) ss=b;
	char *s=strrchr(ss,'.'); if(s) *s=0;
	return strcat(strcat(b,"."),ext);
}

//===================== platform independent Make Directory
int _makeDir(const char * path){
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) return 0;
#if defined(_WIN32)
#if __GNUC__ > 5
	return _mkdir(path);
#else
	return mkdir(path);
#endif
#else
	mode_t mode=S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH;
	return mkdir(path, mode); // notice that 777 is different than 0777
#endif
}
//===================== platform independent Make Directory
void makeDir(const char *path){
	char b[2048];
	parseTilda(b,path);
	char *s=b+strlen(b)-1;
	if(*s=='/') *s=0;
	for(char *s=b; (s=strchr(s+1,'/'))!=0;){
		*s=0;
		if(_makeDir(b))
			errorExit("Can not create directory %s\n",b);
		*s='/';
	}
	_makeDir(b);
}
//================== make path - add '/' if necessary
char* makePath(char* pt){
	if(pt==0) return pt;
	char b[2048];
	char *s=pt+strlen(pt)-1;
	if(*s=='/') *s=0;
	return strdup(strcat(strcpy(b,pt),"/"));
}

//====================== Lock file in platform - independent manner
void flockFile(FILE *f){
#if defined(_WIN32)
	return;
#else
	flockfile(f);
#endif
}
void funlockFile(FILE *f){
#if defined(_WIN32)
	return;
#else
	funlockfile(f);
#endif
}


//=================== Check if given file exists
bool fileExists(const char *fname){
	bool fg=false;						// The file do not exist. The header should be writen.
	FILE *f=gopen(fname,"r");	// check if file exists
	if(f!=0) {fg=true; fclose(f);}
	return fg;
}

//=================== Check if given file exists
bool fileExists(const char* path, const char *fname){
	char b[4096];
	makeFileName(b,path,fname);
	return fileExists(b);
}
//==================== check if the file exists
bool fileExists(const char* path, const char *fname, const char *ext){
	char b[4096];
	makeFileName(b,path,fname,ext);
	return fileExists(b);
}
//====================
unsigned long getFileTime(const char *fname){
	struct stat mystat;
	stat(fname,&mystat);
	return mystat.st_mtime;
}
//=================== extract file name
const char *getExt(const char *fname){
	const char *s=strrchr(fname,'/');
	if(s==0) s=fname;
	s=strrchr(s,'.');
	if(s==0) return 0;
	return s+1;
}
//=================== extract fname wothout extension
char *getFnameWithoutExt(char *buf, char *fname){
	char *s;
	s=strrchr(fname,'/'); if(s==0) s=fname; else s++;
	strcpy(buf,s);

	s=strrchr(buf,'.'); if(s) *s=0;
	return buf;
}

//================= Create directories
void makeDirs(){
	if(profPath!=0) makeDir(profPath);
	else profPath=strdup("./");
	if(resPath!=0) makeDir(resPath);
	else resPath=strdup("./");
	if(trackPath!=0) makeDir(trackPath);
	else trackPath=strdup("./");
}



//============================================================
//========================    Memory    ======================
//============================================================
void zfree(void *a, const char* b){
	if(a) free(a); else if(b) writeLogErr("double free %s\n",b);
}
void *xmalloc(size_t n, const char *err){
	void *a=malloc(n);
	if(a==0){
		if(err==0)
			errorExit("can't allocate memory: %li",n);
		else
			errorExit("can't allocate memory. %s: %li",err,n);
	}
	return a;
}
void *xrealloc(void *a, size_t n, const char * err){
	a=realloc(a,n);
	if(a==0){
		if(err==0)
			errorExit("can't allocate memory: %li",n);
		else
			errorExit("can't allocate memory. %s: %li",err,n);
	}
	return a;
}

//============================================================
//========================    Random    ======================
//============================================================
//=====================================================
inline int longRand(){
	int x=rand();
	if(RAND_MAX > 0xfffff) {return x;}
	return (rand()<<15)|x;
}

double drand(){   /* uniform distribution, (0..1] */
	double x=(longRand()+1.0)/(LRAND_MAX+1.0);
  return x;
}

double rGauss(){
	double phi=drand() * 2 * PI, r=0;
	while(r==0) r=drand();
	double rr=sqrt(-2.*log(r))*sin(phi);
	return rr;
}

// random gaussian variable with given mean anf std deviation
double rGauss(double e, double sigma){
	return rGauss()*sigma+e;
}

// random integer in given interval
unsigned long randInt(unsigned long n){
	int k=longRand();
	double x=(double)(k)/(double)(LRAND_MAX);
	unsigned long rn=(unsigned long)(x*n);
	return rn;
}

//============================================================
//===================    Dynamic  Histogram    ===============
//============================================================
DinHistogram::DinHistogram(int ll){
	l=ll;
	getMem(hist[0],l,"Dinamic histogram 0");
	getMem(hist[1],l,"Dinamic histogram 1");
	getMem(cnts[0],l,"Dinamic histogram 3");
	getMem(cnts[1],l,"Dinamic histogram 4");
	clear();
}
DinHistogram::~DinHistogram(){
	xfree(hist[0],"Dinamic histogram 0");
	xfree(hist[1],"Dinamic histogram 1");
	xfree(cnts[0],"Dinamic histogram 3");
	xfree(cnts[1],"Dinamic histogram 4");
}

void DinHistogram::clear(){
	n[0]=n[1]=0;					//number of observations
	min=1.e+200; max=-min;			//min max values
	bin=hMin=hMax=0;				//bin width, max-min value in the histogram
	e[0]=e[1]=sd[0]=sd[1]=0;		//mean and std deviation
	zeroMem(hist[0],l);
	zeroMem(hist[1],l);
	zeroMem(cnts[0],l);
	zeroMem(cnts[1],l);
}


int DinHistogram::getIdx(double value){ //get index by value
	if(bin==0) return 0;
	if(value==hMax) return l-1;
	return (int)((value-hMin)/bin);
}
double DinHistogram::getValue(int idx){	//get value by index
	return hMin+idx*bin+bin/2;
}

double DinHistogram::getNormValue(int idx){
	double v=getValue(idx);
	v=(v-min)/(max-min);
	return v;
}


int DinHistogram::compress2Left(double value){  //Compress the histogram to the Left
	for(int i=0, j=0; i<l; i+=2, j++){			// Compress the values
		cnts[0][j]=cnts[0][i]+cnts[0][i+1];
		cnts[1][j]=cnts[1][i]+cnts[1][i+1];
	}
	for(int i=l/2; i<l; i++) cnts[0][i]=cnts[1][i]=0;	// Clear new space
	bin*=2;												// redefine bin size
	hMax=hMin+bin*l;									// redefine boundaries
	return getIdx(value);
}

int DinHistogram::compress2Right(double value){	//Compress the histogram to the Right
	for(int i=l-1,j=l-1; i > 0; i-=2,j--){		// Compress the values
		cnts[0][j]=cnts[0][i]+cnts[0][i-1];
		cnts[1][j]=cnts[1][i]+cnts[1][i-1];
	}
	for(int i=0; i<l/2; i++) cnts[0][i]=cnts[1][i]=0;	// Clear new space
	bin*=2;												// redefine bin size
	hMin=hMax-bin*l;									// redefine boundaries
	return getIdx(value);
}

void DinHistogram::addStat(double value, int count, int type){		//add the value to the statistics
	n[type]+=count; e[type]+=value; sd[type]+=value*value;	//count, mean, std dev.
	if(value > max) max=value;
	if(value < min) min=value;
}

void DinHistogram::add(double value, int type){			// add the value to the histogram
	add(value,1,type);
}

void DinHistogram::add(double value, int count, int type){			// add the value to the histogram
	if(count==0) return;
	if(n[0]==0 && n[1]==0){								// the histogram is empty
		cnts[type][0]=count; addStat(value,count,type);			// set the value
		hMin=hMax=value;								// define boundaries
		return;
	}
	if(hMin==hMax){										// the histogram contains only single bin
		if(value==hMin){								// new value equal to the single bin
			cnts[type][0]+=count; addStat(value,count,type);		//
			return;
		}
		else if(value > hMin){							// the new value is right to single bin
			cnts[type][l-1]+=count; addStat(value,count,type);		// the new value defines the right bin
			hMax=value;								// redefine the boundaries
		}
		else{											// the new value is less than single bin
			cnts[0][l-1]=cnts[0][0];					// Put the old value to the right end of the histogram
			cnts[1][l-1]=cnts[1][0];					//
			cnts[type][0]+=count; addStat(value,count,type);		// Put the new value to the left bin
			hMin=value;								// redefine the boundaries
		}
		bin=(hMax-hMin)/l;								// Define the bin
		return;
	}
	int i=getIdx(value);								// The histogram contains some values
	while(i < 0){i=compress2Right(value);}				// The new value is less than the first bin
	while(i >=l){i=compress2Left(value);}				// The new value is grater than the last bin
	cnts[type][i]+=count; addStat(value,count,type);				// Put the new value
}

void DinHistogram::fin(){								// Normalize and calculate the statistics
	for(int t=0; t<2; t++){
		for(int i=0; i<l; i++) hist[t][i]=(double) cnts[t][i]/n[t]/bin;
		e[t]/=n[t];
		sd[t]=(sd[t]-e[t]*e[t]*n[t])/(n[t]-1);
		sd[t]=sqrt(sd[t]);
	}
}

void DinHistogram::print(FILE* f){						// print the histogram
	fprintf(f,"#  min=%.3f max=%.3f \n",min,max);
	fprintf(f,"#  hMin=%.3f  hMax=%.3f bin=%.3f\n",hMin,hMax,bin);
	fprintf(f,"#  e0=%.3f sd0=%.3f n0=%i\n",e[0],sd[0],n[0]);
	fprintf(f,"#  e1=%.3f sd1=%.3f n1=%i\n",e[1],sd[1],n[1]);
	for(int i=getIdx(min); i<getIdx(max); i++){
		double h0=hist[0][i];
		double h1=hist[1][i];
		fprintf(f,"%.3f\t\%6i\t%.6f\t\%6i\t%.6f\n",getValue(i),cnts[0][i],h0,cnts[1][i],h1);
	}
}


//===================== Normalize the function to mean and sigma
double norm(double *x, int l){
	double d=0,e=0,dd,ee;
	for(int i=0; i<l; i++){d+=x[i]*x[i]; e+=x[i];}
	ee=e/l; d=d*l-e*e; dd=d/((l-1)*l);
	if(dd<0) {dd=0;} dd=sqrt(dd);
	if(dd <= ee*ee*1.e-5) {
		return 0;}

	for(int i=0; i<l; i++) x[i]=(x[i]-ee)/dd;
	return dd;
}

//======================================================================
int nearPow2(int n, int &i){
	int nn=1;
	for(i=0; i<30; i++){
		if(nn >= n) return nn;
		nn*=2;
	}
	return nn;
}
int nearPow2(int n){
	int z;
	return nearPow2(n,z);
}

int nearFactor(int n){
	int qMin;
	int pow2=nearPow2(n);
	qMin=pow2;
	int k3=1,k35=1;
	for(k3=1; k3<pow2; k3*=3){
		for(k35=k3; k35 < pow2; k35*=5){
				int p2=nearPow2(k35);
				int qq=pow2*k35/p2;
				if(qq>=n && qq<qMin) qMin=qq;
		}
	}
return qMin;
}

//===================== convert text flag to a binary
int getFlag(char*s){
	int fg=0;
	if(		keyCmp(s,"1")==0 || keyCmp(s,"YES")==0 || keyCmp(s,"ON" )==0) {fg=1;}
	else if(keyCmp(s,"0")==0 || keyCmp(s,"NO")==0  || keyCmp(s,"OFF")==0) {fg=0;}
	else fg=-1;
	return fg;
}
//=================================================================
//============================= File list =========================
//=================================================================
int   fileId=0;

void addFile(char* fname, int id){
	fname=trim(fname);
	if(strlen(fname)==0) return;

	files[nfiles].fname=strdup(fname);
	files[nfiles].id=fileId;
	nfiles++;
}

void addFile(char* fname){
	if(nfiles > 256) errorExit("too many input files\n");
	char b[4096], *s;
	strcpy(b,fname); s=strrchr(b,'.'); if(s) s++;
	if(s && (keyCmp(s,"lst")==0 || keyCmp(s,"list")==0)){
		FILE *f=0;
		if(fileExists(fname)) f=xopen(fname,"rt");
		else{
			makeFileName(b,trackPath,fname);
			f=xopen(b,"rt");
		}
		for(;(s=fgets(b,sizeof(b),f))!=0;){
			strtok(b,"\r\n#");
			s=trim(b);
			if(strlen(s)==0 || *s=='#') continue;
			addFile(s, fileId);
		}
		fclose(f); fileId++;
		return;
	}
	else{
		addFile(fname, fileId); fileId++;
	}
}


unsigned int hashx(unsigned int h,char c){
	return h+(c-32)+1234;
}
unsigned int hashx(unsigned int h,const char *s){
	if(s==0) return h;
	for(;*s;s++) h=hashx(h,*s);
	return h;
}
unsigned int hashx(unsigned int h,unsigned int x){
	return h*3+x;
}
unsigned int hashx(unsigned int h,int x){
	return h*3+x;
}
unsigned int hashx(unsigned int h,long x){
	return h*3+x;
}
unsigned int hashx(unsigned int h,float c){
	unsigned int f; memcpy(&f,&c,sizeof(float));
	return hashx(h,f);
}
unsigned int hashx(unsigned int h,double c){
	float f=(float) c;
	return hashx(h,f);
}

void makeId(){
	id=0;
	id=hashx(id,chromFile);
	id=hashx(id,flankSize);
	id=hashx(id,kernelType);
	id=hashx(id,binSize);
	id=hashx(id,nShuffle);
	id=hashx(id,maxZero);
	id=hashx(id,maxNA0);
	id=hashx(id,kernelSigma);
	id=hashx(id,wSize);
	id=hashx(id,wStep);
	id=hashx(id,threshold);
	id=hashx(id,outFile);
	id=hashx(id,mtime());
}
//======================================================================
BufFile::~BufFile(){
	if(f!=0) fclose(f);
	if(buffer) xfree(buffer,"buff file");
}

void BufFile::init(const char *fname){
	f=fopen(fname,"rb"); buffer=0;
	getMem0(buffer,SG_BUFSIZ+SG_BUFEXT,"err");
	int n=fread(buffer,1,SG_BUFSIZ,f);
	if(n <= 0) {curString=0;}
	else {curString=buffer; buffer[n]=0;}
}


char *BufFile::getString(){
	if(curString==0) return 0;
	char *s0=curString;
	while(isspace(*s0)) s0++;
	char *ss=strchr(curString,'\n');
	if(ss==0){
		strcpy(buffer,curString);
		char *bb=buffer+strlen(curString);
		int n=fread(bb,1,SG_BUFSIZ,f);
		if(n <= 0) {curString=0; return s0;}
		else {curString=buffer; bb[n]=0;}
		return(getString());
	}
	curString=ss+1;
	while(isspace(*ss)) *ss--=0;
//	*ss=0;
	return s0;
}
//======================================================================


