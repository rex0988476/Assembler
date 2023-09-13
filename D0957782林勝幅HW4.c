#include <string.h>
#include <stdlib.h>//{
#include <stdio.h>//}
#include "2-optable.c"

/* Public variables and functions */
#define	ADDR_SIMPLE			0x01
#define	ADDR_IMMEDIATE		0x02
#define	ADDR_INDIRECT		0x04
#define	ADDR_INDEX			0x08

#define	LINE_EOF			(-1)
#define	LINE_COMMENT		(-2)
#define	LINE_ERROR			(0)
#define	LINE_CORRECT		(1)
//{
int Mfrontsum=0;
int Mlenth=0;
char label[1000][LEN_SYMBOL];
char objtext[1000][1000];
int objtextlen=0;
int endrec;
int pass2=0;
int labelLen=0;
int labelAddress[1000]={0};
int locctr[1000]={0};
int isLabel[1000]={0};
int sum=0;
int newl=0;
int special=0;
int is_startline=1;
int startAddress=0;
int addLater=0;
int target_label_add=0;
int label_found=0;
char op[9],x,b,p,e,disp[4],addr[6],r1,r2;
char bop1[8],bop2[16],bop3[24],bop4[32];
int can_base=0;
char filename[7];
char strstartadd[6];
int is_error=0;
char strProgramLength[6];
char objline[69];
char objlineafter[65];
char objstartadd[6];
char objlinelen[2];
char objob[9];
int nopc=0;
unsigned PC=0;
unsigned X=0;
unsigned B=0;
unsigned addressdiff;
unsigned BDISP=0;
int PCDISP=0;
unsigned TA=0;
int is_base_first=1;
int is_nobase_first=1;
char Rtab[10][2]={{"A"},{"X"}, {"L"}, {"B"}, {"S"}, {"T"}, {"F"}, {"PC"}, {"SW"}};
char Rnum[10]="012345689";
int PCstore=0;
//}
typedef struct
{
	char		symbol[LEN_SYMBOL];
	char		op[LEN_SYMBOL];
	char		operand1[LEN_SYMBOL];
	char		operand2[LEN_SYMBOL];
	unsigned	code;
	unsigned	fmt;
	int			is_direct;
	unsigned	addressing;	
	char		lobjob[9];
	int			llocctr;
	int			objlen;
} LINE;
LINE		line[1000];
int process_line(LINE *line);
/* return LINE_EOF, LINE_COMMENT, LINE_ERROR, LINE_CORRECT and Instruction information in *line*/

/* Private variable and function */

FILE		*RASM_fp;//{
FILE	 *RASM_open(char *fname)
{
	RASM_fp = fopen(fname, "r");
	return (RASM_fp); 
}

void RASM_close(void)
{
	fclose(RASM_fp);
}

char program_line[1000][100];
int get_line(void){
	int program_length=0;
	for(program_length=0;;program_length++){
		if(fscanf(RASM_fp,"%[^\n]%*c",program_line[program_length])==0){
			fgets(program_line[program_length],2,RASM_fp);
		}
		if(feof(RASM_fp))break;
	}
	return program_length;
}//}


void init_LINE(LINE *line)
{
	line->symbol[0] = '\0';
	line->op[0] = '\0';
	line->operand1[0] = '\0';
	line->operand2[0] = '\0';
	line->code = 0x0;
	line->fmt = 0x0;
	line->addressing = ADDR_SIMPLE;
}

int process_line(LINE *line)
/* return LINE_EOF, LINE_COMMENT, LINE_ERROR, LINE_CORRECT */
{
	char		buf[LEN_SYMBOL];
	int			c;
	int			state;
	int			ret;
	Instruction	*op;
	
	c = ASM_token(buf);		/* get the first token of a line */
	if(c == EOF)
		return LINE_EOF;
	else if((c == 1) && (buf[0] == '\n')){
		return LINE_COMMENT;
	}	/* blank line */
		
	else if((c == 1) && (buf[0] == '.'))	/* a comment line */
	{
		do
		{
			c = ASM_token(buf);
		} while((c != EOF) && (buf[0] != '\n'));
		return LINE_COMMENT;
	}
	else
	{
		init_LINE(line);
		ret = LINE_ERROR;
		if(strcmp(line->op,"NOBASE")==0)printf("else(first)");
		state = 0;
		while(state < 8)
		{
			switch(state)
			{
				case 0:
				case 1:
				case 2:
					
					
					if(pass2==0){
						op = is_opcode(buf);
						if (op == NULL&&newl==1&&strcmp(buf,"+")!=0){
						newl=0;
						strcpy(label[labelLen],buf);
						labelAddress[labelLen]=sum;
						labelLen++;
						}
						else{
							newl=0;
						}
					}
					
					op = is_opcode(buf);
					if((state < 2) && (buf[0] == '+'))	/* + */
					{
						line->fmt = FMT4;
						state = 2;
					}
					else	if(op != NULL)	/* INSTRUCTION */
					{
						strcpy(line->op, op->op);
						//printf("op[%s]",op->op);
						//{
						if(pass2==0){
						if(strcmp(op->op,"WORD")==0)addLater=3;
						if(strcmp(op->op,"RESW")==0){
							special=1;
						}
						if(strcmp(op->op,"RESB")==0){
							special=2;
						}
						if(strcmp(op->op,"BYTE")==0){
							special=3;
						}
						if(strcmp(op->op,"START")==0||is_startline==1){
							labelLen--;
							is_startline=0;
							if(strcmp(op->op,"START")==0){
								special=4;
							}
							if(strlen(label[labelLen])>6){
								is_error=1;
								printf("strlen(label[labelLen])>6");
								break;
							}
							strcpy(filename,label[labelLen]);
							int filenamediff=strlen(filename);
							while(filenamediff<6){
								filename[filenamediff]=' ';
								filenamediff++;
							}
							filename[6]='\0';
							strcpy(label[labelLen],"");
							labelAddress[labelLen]=0;
						}	
						}
						//}
						line->code = op->code;
						state = 3;
						if(line->fmt != FMT4)
						{
							line->fmt = op->fmt & (FMT1 | FMT2 | FMT3);
						}
						else if((line->fmt == FMT4) && ((op->fmt & FMT4) == 0)) /* INSTRUCTION is FMT1 or FMT 2*/
						{	/* ERROR 20210326 added */
							printf("ERROR at token %s, %s cannot use format 4 \n", buf, buf);
							ret = LINE_ERROR;
							if(strcmp(line->op,"NOBASE")==0)printf("(line->fmt == FMT4) && ((op->fmt & FMT4) == 0)");
							state = 7;		/* skip following tokens in the line */
						}
					}				
					else	if(state == 0)	/* SYMBOL */
					{
						strcpy(line->symbol, buf);
						//printf("symb[%s]",buf);
						state = 1;
					}
					else		/* ERROR */
					{
						//printf("ERROR at token %s\n", buf);
						//printf("[E]");//{}
						ret = LINE_ERROR;
						if(strcmp(line->op,"NOBASE")==0)printf("case012 else");
							
						state = 7;		/* skip following tokens in the line */
					}
					break;	
				case 3:
					if(line->fmt == FMT1 || line->code == 0x4C || strcmp(line->op,"NOBASE")==0)	/* no operand needed */
					{
						if(c == EOF || buf[0] == '\n')
						{
							ret = LINE_CORRECT;
							state = 8;
						}
						else		/* COMMENT */
						{
							ret = LINE_CORRECT;
							state = 7;
						}
					}
					else
					{
						if(c == EOF || buf[0] == '\n')
						{
							ret = LINE_ERROR;
							if(strcmp(line->op,"NOBASE")==0){
								printf("right here");
								printf("c(%d)buf(%c)",c,buf[0]);
							}
							
							state = 8;
						}
						else	if(buf[0] == '@' || buf[0] == '#')
						{
							line->addressing = (buf[0] == '#') ? ADDR_IMMEDIATE : ADDR_INDIRECT;
							state = 4;
						}
						else	/* get a symbol */
						{
							op = is_opcode(buf);
							if(op != NULL)
							{
								//printf("Operand1 cannot be a reserved word\n");
								//printf("[O1]");//{}
								ret = LINE_ERROR;
								if(strcmp(line->op,"NOBASE")==0)printf("op != NULL");
							
								state = 7; 		/* skip following tokens in the line */
							}
							else
							{
								strcpy(line->operand1, buf);
								//{
								//printf("op1-1[%s]",line->operand1);
								if(pass2==1){
									int i;
									for(i=0;i<labelLen;i++){
										if(strcmp(buf,label[i])==0){
											//printf("{%s found}",buf);
											target_label_add=labelAddress[i];
											label_found=1;
											break;
										}
									}
								}
								
								
								if(pass2==0){
								if(special==1){//RESW 
									special=0;
									int n=atoi(buf);
									addLater=(3*n);
								}
								if(special==2){//RESB
									special=0;
									int n=atoi(buf);
									
									addLater=n;
								}
								if(special==3){//BYTE
									special=0;
									char target[LEN_SYMBOL];
									int i;
									for(i=2;i<strlen(buf)-1;i++){
										target[i-2]=buf[i];
									}
									target[i-2]='\0';
									if(buf[0]=='C'){
										int n=strlen(target);
										addLater=n;
									}
									if(buf[0]=='X'){
										int n=strlen(target);
										addLater=(int)(n/2);
									}
								}
								if(special==4){//STRAT
									special=0;
									//printf("atoi [%s]",buf);
									int bufi,startaddi;
									if(strlen(buf)>6){
										is_error=1; 
										printf("strlen(buf)>6");
										break;
									}
									for(bufi=strlen(buf)-1,startaddi=5;bufi>=0;bufi--,startaddi--){
										strstartadd[startaddi]=buf[bufi];
									}
									while(startaddi>=0){
										strstartadd[startaddi]='0';
										startaddi--;
									}
									startAddress=atoi(buf);
									sum=startAddress;
								}	
								}
								//}
								state = 5;
							}
						}
					}			
					break;		
				case 4:
					op = is_opcode(buf);
					if(op != NULL)
					{
						//printf("Operand1 cannot be a reserved word\n");
						//printf("[O1]");//{}
						ret = LINE_ERROR;
							if(strcmp(line->op,"NOBASE")==0)printf("op != NULL 4");
							
						state = 7;		/* skip following tokens in the line */
					}
					else
					{
						strcpy(line->operand1, buf);
						//{
						//printf("op1-2[%s]",buf);
						if(pass2==1){
									int i;
									for(i=0;i<labelLen;i++){
										if(strcmp(buf,label[i])==0){
											//printf("{%s found}",buf);
											target_label_add=labelAddress[i];
											label_found=1;
											break;
										}
									}
								}
						//}
						state = 5;
					}
					break;
				case 5:
					if(c == EOF || buf[0] == '\n')
					{
						ret = LINE_CORRECT;
						state = 8;
					}
					else if(buf[0] == ',')
					{
						state = 6;
					}
					else	/* COMMENT */
					{
						ret = LINE_CORRECT;
						state = 7;		/* skip following tokens in the line */
					}
					break;
				case 6:
					if(c == EOF || buf[0] == '\n')
					{
						ret = LINE_ERROR;
						if(strcmp(line->op,"NOBASE")==0)printf("c == EOF || buf[0] == '\n'");
						state = 8;
					}
					else	/* get a symbol */
					{
						op = is_opcode(buf);
						if(op != NULL)
						{
							//printf("Operand2 cannot be a reserved word\n");
							//printf("[O2]");//{}
							ret = LINE_ERROR;
							if(strcmp(line->op,"NOBASE")==0)printf("op != NULL");
							state = 7;		/* skip following tokens in the line */
						}
						else
						{
							if(line->fmt == FMT2)
							{
								strcpy(line->operand2, buf);
								//printf("op1[%s]op2[%s]",line->operand1,buf);
								ret = LINE_CORRECT;
								state = 7;
							}
							else if((c == 1) && (buf[0] == 'x' || buf[0] == 'X'))
							{
								line->addressing = line->addressing | ADDR_INDEX;
								//printf("addr[%d]",line->addressing);
								ret = LINE_CORRECT;
								state = 7;		/* skip following tokens in the line */
							}
							else
							{
								//printf("Operand2 exists only if format 2  is used\n");
								//printf("[O2]");//{}
								ret = LINE_ERROR;
								if(strcmp(line->op,"NOBASE")==0)printf("else");
								state = 7;		/* skip following tokens in the line */
							}
						}
					}
					break;
				case 7:	/* skip tokens until '\n' || EOF */
					if(c == EOF || buf[0] =='\n')
						state = 8;
					break;										
			}
			if(state < 8)
				c = ASM_token(buf);  /* get the next token */
		}
		return ret;
	}
}

//printLine
int line_len=0;
int line_count=1;
void printLine(void){
	int i=0;
	printf("---------------------------------------\n");
	for(i=0;i<line_len;i++){
		/*if(line[i].fmt!=0x0C&&line[i].fmt!=0x0E)*/printf("pc(%06X)loc(%06X) : %12s %12s(%06X) %12s,%12s obj<%s>           (FMT=%X, ADDR=%X)\n",locctr[i+1], locctr[i], line[i].symbol, line[i].op,line[i].code, line[i].operand1, line[i].operand2,line[i].lobjob, line[i].fmt, line[i].addressing);
	}
	printf("---------------------------------------\n");
}
void getlocctrtoline(void){
	int gi;
	for(gi=0;gi<line_len;gi++){
		line[gi].llocctr=locctr[gi];
	}
}

void htob(unsigned hex){
    int i=0;
	while(hex>0){
        //printf("%d\n", hex%2);
		op[7-i]=((hex%2)+48);
		hex/=2;
		i++;
    }
    while(7-i>=0){
        op[7-i]='0';
        i++;
    }
}



void bitLineInit(void){
	strcpy(bop1,"");
	strcpy(bop2,"");
	strcpy(bop3,"");
	strcpy(bop4,"");
}
void bitInit(void){
	x='0';
	b='0';
	p='1';
	e='0';
	addressdiff=0;
	BDISP=0;
	PCDISP=0;
	TA=0;
	strcpy(op,"");
	r1='0';
	r2='0';
	strcpy(disp,"");
	strcpy(addr,"");
}

int is_const(char *str){
	int imi,im_is_const=1;
	for(imi=0;imi<strlen(str);imi++){
		if(str[imi]<48 || str[imi]>57){
			return 0;
		}
	}
	if(im_is_const==1){
		return 1;
	}
	return 0;
}

int findLabAdd(int index){
	int i;
	for(i=0;i<labelLen;i++){
		if(strcmp(line[index].operand1,label[i])==0){
			return i;
		}
	}
	return -1;
}

void ADDR(int index){//op(8)+x
	unsigned addressing=line[index].addressing;
	if(addressing>=8){
		addressing-=8;
		x='1';
	}
	if(addressing>=4){
		addressing-=4;
		op[6]='1';
		op[7]='0';
	}
	if(addressing>=2){
		addressing-=2;
		op[6]='0';
		op[7]='1';
	}
	if(addressing>=1){
		addressing-=1;
		op[6]='1';
		op[7]='1';
	}
}

void is_base_func(int index){
	//printf("baseop[%s]",line[index].op);
	if(strcmp(line[index].op,"BASE")==0 || can_base==1){
		printf("[!]");
		if(is_base_first==1){
			if(is_const(line[index].operand1)==1){
				B=atoi(line[index].operand1);
			}
			else{
				int bindex=findLabAdd(index);
				B=labelAddress[bindex];
			}
			printf("B[%06X]",B);
			is_base_first=0;
			PCstore=locctr[index+1];
			is_nobase_first=1;
		}
		can_base=1;	

	}
	if(strcmp(line[index].op,"NOBASE")==0 || can_base==0){
		if(is_nobase_first==1){
			is_nobase_first=0;
			is_base_first=1;
			PC=PCstore;
		}
		can_base=0;
	}
	//printf("\n");
}

int is_end(int index){
	if(strcmp(line[index].op,"END")==0){
		return 1;
	}
	return 0;
}


int word_func(int index){
	if(is_const(line[index].operand1)==1){
		//printf("[%s]",line[index].operand1);
		int bbuf=atoi(line[index].operand1);
		itoa(bbuf,objob,16);

		int lendiff=6-strlen(objob);
		if(lendiff>0){
			int objobi;
			for(objobi=strlen(objob)-1;objobi>=0;objobi--){
				objob[objobi+lendiff]=objob[objobi];
			}
			lendiff--;
			while(lendiff>=0){
				objob[lendiff]='0';
				lendiff--;
			}
			objob[6]='\0';
		}

		return 1;
	}
	else{
		printf("error...word_func(%s)",line[index].operand1);
		return 0;
	}
	return 0;
}

int byte_func(int index){
	int iint[4];
	char schar[7];
	int bytei;
	if(line[index].operand1[0]=='C'){
		//printf("[%s]",line[index].operand1);
		for(bytei=0;bytei<3&&bytei+3<strlen(line[index].operand1);bytei++){
			iint[bytei]=line[index].operand1[bytei+2];
		}
		iint[bytei]='\0';
		char strbuf[3];
		strcpy(objob,"");
		for(bytei=0;bytei<3;bytei++){
			itoa(iint[bytei],strbuf,16);
			strcat(objob,strbuf);
		}
		return 1;
	}
	else if(line[index].operand1[0]=='X'){
		//printf("[%s]",line[index].operand1);
		if((strlen(line[index].operand1)-3)%2!=0){
			printf("X len ERROR...");
			return 0;
		}
		for(bytei=0;bytei<6&&bytei+3<strlen(line[index].operand1);bytei++){
			schar[bytei]=line[index].operand1[bytei+2];
			
		}
		//printf("schar(%s)(%d)",schar,strlen(schar));
		schar[bytei]='\0';
		strcpy(objob,schar);
		return 1;
	}
	else{
		printf("error...byte_func");
		return 0;
	}
	return 0;
}

int fmt1(int index){
	printf("-fmt1-");
	itoa(line[index].code,objob,16);
	//printf("obj<%s>\n",objob);
	return 1;
}

int fmt2(int index){
	printf("-fmt2-");
	int ri,fmt2_is_error=1;
	for(ri=0;ri<9;ri++){
		if(strcmp(line[index].operand1,Rtab[ri])==0){
			r1=Rnum[ri];
			fmt2_is_error=0;
		}
		if(strcmp(line[index].operand2,Rtab[ri])==0){
			r2=Rnum[ri];
		}
	}
	if(fmt2_is_error==1){
		return -1;
	}
	itoa(line[index].code,objob,16);
	objob[2]=r1;
	objob[3]=r2;
	objob[4]='\0';
	printf("op1(%s)(%c),op2(%s)(%c)",line[index].operand1,r1,line[index].operand2,r2);
	return 1;
}

int fmt3(int index){//bpe+disp
	e='0';
	printf("-fmt3-");
	if(x=='1'){
		addressdiff+=X;
		printf("-index-");
	}
	int is_imm=0;
	if(op[6]=='0'&&op[7]=='1'&&is_const(line[index].operand1)==1){//im
		b='0';
		p='0';
		is_imm=1;
		printf("-imm-");
	}
	if(is_imm==1){
		TA=atoi(line[index].operand1);
		//printf("TA(%X)",TA);
		itoa(TA,disp,16);
	}
	if(is_imm==0){
		int labIndex=findLabAdd(index);
		if(labIndex==-1&&strcmp(line[index].op,"RSUB")!=0){
			printf("ERROR occur...labIndex(-1)");
			return 0;
		}
		if(strcmp(line[index].op,"RSUB")!=0){
			TA=labelAddress[labIndex];
		}
		if(strcmp(line[index].op,"RSUB")==0){
			strcpy(objob,"4F0000");
			return 1;
		}
		if(((TA-(PC+addressdiff))<=2047||(TA-(PC+addressdiff))>=-2048)){
			addressdiff+=PC;
			PCDISP=TA-addressdiff;
			b='0';
			p='1';
			if(PCDISP<0){
				PCDISP=4096+PCDISP;
				//printf("pc<0(%06X)",PCDISP);
			}
			printf("-pc-(%06X)=(%06X)-(%06X),PC:(%06X)",PCDISP,TA,addressdiff,PC);
			//printf("")
			nopc=0;
			itoa(PCDISP,disp,16);
		}
		else if(can_base==1&&((TA-(B+addressdiff))>=0 && (TA-(B+addressdiff))<=4095)){
			addressdiff+=B;
			BDISP=TA-addressdiff;
			b='1';
			p='0';
			printf("-base-");
			nopc=1;
			itoa(BDISP,disp,16);
		}
		else{
			printf("ERROR occur...disp(%d)(%06X)(%06X)(%06X)[%06X]\n",can_base,TA,B,addressdiff,(TA-(B+addressdiff)));
			
			return 0;
		}
	}
	
	int lendiff=3-strlen(disp);
	if(lendiff>0){
		int dispi;
		for(dispi=strlen(disp)-1;dispi>=0;dispi--){
			disp[dispi+lendiff]=disp[dispi];
		}
		lendiff--;
		while(lendiff>=0){
			disp[lendiff]='0';
			lendiff--;
		}
		disp[3]='\0';
	}
	//op[8]='\0';
	strcat(bop3,op);
	bop3[8]=x;
	bop3[9]=b;
	bop3[10]=p;
	bop3[11]=e;
	char *stop;
	int dec=strtol(bop3,&stop,2);
	itoa(dec,objob,16);
	int obdiff=3-strlen(objob);
	if(obdiff>0){
		int objobi;
		for(objobi=strlen(objob)-1;objobi>=0;objobi--){
			objob[objobi+obdiff]=objob[objobi];
		}
		obdiff--;
		while(obdiff>=0){
			objob[obdiff]='0';
			obdiff--;
		}
		objob[3]='\0';
	}
	strcat(objob,disp);
	return 1;
}


int fmt4(int index){//bpe+addr
	line[index].is_direct=1;
	e='1';
	b='0';
	p='0';
	printf("-fmt4-");
	if(x=='1'){
		addressdiff+=X;
		printf("-index-");
	}
	int is_imm=0;
	if(op[6]=='0'&&op[7]=='1'&&is_const(line[index].operand1)==1){//im
		is_imm=1;
		line[index].is_direct=0;
		printf("-imm-");
	}
	if(is_imm==1){
		TA=atoi(line[index].operand1);
		//printf("TA(%X)",TA);
		itoa(TA,disp,16);
	}
	else{
		int labIndex=findLabAdd(index);
		if(labIndex==-1){
			printf("ERROR occur...labIndex(-1)");
			return 0;
		}
		TA=labelAddress[labIndex];
	}

	itoa(TA,addr,16);
	int lendiff=5-strlen(addr);
	if(lendiff>0){
		int addri;
		for(addri=strlen(addr)-1;addri>=0;addri--){
			addr[addri+lendiff]=addr[addri];
		}
		lendiff--;
		while(lendiff>=0){
			addr[lendiff]='0';
			lendiff--;
		}
		addr[5]='\0';
	}
	//op[8]='\0';
	strcat(bop3,op);
	bop3[8]=x;
	bop3[9]=b;
	bop3[10]=p;
	bop3[11]=e;
	char *stop;
	int dec=strtol(bop3,&stop,2);
	itoa(dec,objob,16);
	int obdiff=3-strlen(objob);
	if(obdiff>0){
		int objobi;
		for(objobi=strlen(objob)-1;objobi>=0;objobi--){
			objob[objobi+obdiff]=objob[objobi];
		}
		obdiff--;
		while(obdiff>=0){
			objob[obdiff]='0';
			obdiff--;
		}
		objob[3]='\0';
	}
	strcat(objob,addr);
	return 1;
}


int main(int argc, char *argv[])
{
	int			i,c;
	char		buf[LEN_SYMBOL];
	if(argc < 2)
	{
		printf("Usage: %s fname.asm\n", argv[0]);
	}
	else
	{
		if(RASM_open(argv[1]) == NULL)//{
			printf("File not found!!\n");
		else{
			int Plen;
			Plen=get_line();
			RASM_close();
		}//}
		if(ASM_open(argv[1]) == NULL)
			printf("File not found!!\n");
		else
		{	
			int filenamei;
			for(filenamei=0;filenamei<6;filenamei++)filename[filenamei]=' ';
			filename[filenamei]='\0';
			for(line_len=0,line_count = 1 ,i=0,newl=1; (c = process_line(&line[i])) != LINE_EOF; line_count++,line_len++,newl=1,i++)
			{
				newl=1;
				if(c == LINE_ERROR){
					locctr[line_count-1]=sum;//{
					//printf("%06X : %s\n",sum,program_line[line_count-1]);//}
//					printf("ERROR\n");
					line[i].fmt=0x0E;
					printf("c == LINE_ERROR");
					printf("pc(%06X)loc(%06X) : %12s %12s(%06X) %12s,%12s (FMT=%X, ADDR=%X)\n",locctr[i+1], locctr[i], line[i].symbol, line[i].op,line[i].code, line[i].operand1, line[i].operand2, line[i].fmt, line[i].addressing);
					is_error=1;
					
					//printf("%03d : Error\n", line_count);
				}
				else if(c == LINE_COMMENT){
					line[i].fmt=0x0C;
					locctr[line_count-1]=sum;//{
					
					//printf("%06X : %s\n",sum,program_line[line_count-1]);//}
//					printf("COMMENT\n");
					//printf("%03d : Comment line\n", line_count);
				}
					
				else{
					locctr[line_count-1]=sum;//{	
//					printf("%06X : %s\n",sum,program_line[line_count-1]);//}
					
					//printf("%03d : %12s %12s %12s,%12s (FMT=%X, ADDR=%X)\n", line_count, line.symbol, line.op, line.operand1, line.operand2, line.fmt, line.addressing);
					
					if(line[i].fmt==0x01){//{
						sum+=1;
					}
					if(line[i].fmt==0x02){
						sum+=2;
					}
					if(line[i].fmt==0x04){
						sum+=3;
					}
					if(line[i].fmt==0x08){
						sum+=4;
					}
					sum+=addLater;
					addLater=0;//}
				}
			}
			if(sum-startAddress>0xFFFFFF){
				printf("sum-startAddress>0xFFFFFF");
				is_error=1;
				
			}
			else {
				strcpy(strProgramLength,"");
				itoa(sum-startAddress,strProgramLength,16);
				int lendiff=6-strlen(strProgramLength);
				if(lendiff>0){
					int strProgramLengthi;
					for(strProgramLengthi=strlen(strProgramLength)-1;strProgramLengthi>=0;strProgramLengthi--){
						strProgramLength[strProgramLengthi+lendiff]=strProgramLength[strProgramLengthi];
					}
					lendiff--;
					while(lendiff>=0){
						strProgramLength[lendiff]='0';
						lendiff--;
					}
					strProgramLength[6]='\0';
				}
				//printf("strpl[%s]",strProgramLength);
			}
			printf("Program length = %06X\n",sum-startAddress);//{
			for(i=0;i<labelLen;i++){
				printf("%12s : %06X\n",label[i],labelAddress[i]);
			}//}
			ASM_close();
			
		}


//pass2
pass2=1;
		if(argc < 3)printf("no output file...");
		else if(ASM_open(argv[1]) == NULL)
			printf("File not found!!\n");
		else if(is_error==1){printf("ERROR occur...");printf("hehe");}
		else
		{
			int i;
			getlocctrtoline();
			for(i=0;i<line_len;i++)
			{
				line[i].is_direct=0;
				is_base_func(i);
				if(is_end(i)==1){
					int addend=findLabAdd(i);
					if(addend==-1){
						printf("end error...");
						return 0;
					}
					endrec=labelAddress[addend];
				}
				if( (line[i].fmt!=0x0C && line[i].fmt!=0x0E)&& ((strcmp("BYTE",line[i].op)==0 || strcmp("WORD",line[i].op)==0)||line[i].fmt!=0x00) ){
					
					//printf("[ob]");
					bitInit();
					bitLineInit();
					//if(i%10==0)printf("[%06X]\n",locctr[i]);
					PC=locctr[i+1];
					if(nopc==0){
						printf("\npc[%06X]",PC);
					}
					if(nopc==1){
						printf("\nb[%06X]",B);
					}
					if(strcmp(line[i].op,"WORD")==0){
						word_func(i);
						printf("obj<%s>",objob);
						strcpy(line[i].lobjob,"");
						strcpy(line[i].lobjob,objob);
						line[i].objlen=strlen(line[i].lobjob)/2;
					}
					if(strcmp(line[i].op,"BYTE")==0){
						byte_func(i);
						printf("obj<%s>",objob);
						strcpy(line[i].lobjob,"");
						strcpy(line[i].lobjob,objob);
						line[i].objlen=strlen(line[i].lobjob)/2;
					}
					if(line[i].fmt==0X01){
						ADDR(i);
						fmt1(i);
						printf("obj<%s>",objob);
						strcpy(line[i].lobjob,"");
						strcpy(line[i].lobjob,objob);
						line[i].objlen=strlen(line[i].lobjob)/2;
					}
					if(line[i].fmt==0X02){
						ADDR(i);
						int f2err=fmt2(i);
						if(f2err==-1){
							printf("ERROR...fmt2");
						}
						printf("obj<%s>",objob);
						strcpy(line[i].lobjob,"");
						strcpy(line[i].lobjob,objob);
						line[i].objlen=strlen(line[i].lobjob)/2;
					}
					if(line[i].fmt==0X04){
						htob(line[i].code);//op8
						ADDR(i);//nix
						fmt3(i);//bpe+disp
						printf("obj<%s>",objob);
						strcpy(line[i].lobjob,"");
						strcpy(line[i].lobjob,objob);
						line[i].objlen=strlen(line[i].lobjob)/2;
						//printf("bop3<%s> dec<%d> obj<%s> opni[%s] x[%c] b[%c] p[%c] e[%c] disp[%s]\n",bop3,dec,objob,op,x,b,p,e,disp);
						//printf("disp[%06X][%s](%06X)(%06X)(%c,%c)\n",DISP,disp,TA,addressdiff,b,p);
					}
					if(line[i].fmt==0X08){
						htob(line[i].code);
						ADDR(i);
						fmt4(i);
						printf("obj<%s>",objob);
						strcpy(line[i].lobjob,"");
						strcpy(line[i].lobjob,objob);
						line[i].objlen=strlen(line[i].lobjob)/2;
					}
					
				}
			}


			//test
			printLine();



			////////////////obj code
			//H
			strcpy(objtext[objtextlen],"H");
			strcat(objtext[objtextlen],filename);
			strcat(objtext[objtextlen],strstartadd);
			strcat(objtext[objtextlen],strProgramLength);
			printf("{%s}\n",objtext[objtextlen]);
			strcat(objtext[objtextlen],"\n");
			objtextlen++;

			//T
			int is_start=1;
			i=0;
			while(strcmp(line[i].lobjob,"")==0){
				i++;
			}
			for(;i<line_len;i++){

				if( strcmp(line[i].lobjob,"")!=0 ){

					if(is_start==1){
						//printf("<%06X>",locctr[i]);
						strcpy(objlineafter,"");
						strcpy(objline,"");
						strcat(objline,"T");
						itoa(line[i].llocctr,objstartadd,16);
						int lendiff=6-strlen(objstartadd);
						if(lendiff>0){
							int objstartaddi;
							for(objstartaddi=strlen(objstartadd)-1;objstartaddi>=0;objstartaddi--){
								objstartadd[objstartaddi+lendiff]=objstartadd[objstartaddi];
							}
							lendiff--;
							while(lendiff>=0){
								objstartadd[lendiff]='0';
								lendiff--;
							}
							objstartadd[6]='\0';
						}
						strcat(objline,objstartadd);
						//printf("objl[%s]\n",objline);
						is_start=0;
					}

					strcat(objlineafter,line[i].lobjob);
					
					if(strlen(objlineafter)+strlen(line[i+1].lobjob)>60){
						is_start=1;
						strcpy(objtext[objtextlen],objline);
						int hex=strlen(objlineafter)/2;
						char str[3];
						itoa(hex,str,16);
						str[2]='\0';
						if(strlen(str)>2){
							printf("objlinelen error...");
							return 0;
						}
						if(strlen(str)==1){
							str[1]=str[0];
							str[0]='0';
						}
						strcpy(objlinelen,str);
						strcat(objtext[objtextlen],objlinelen);
						strcat(objtext[objtextlen],objlineafter);
						printf("{%s}(%d)(%d)\n",objtext[objtextlen],strlen(objlineafter),strlen(objtext[objtextlen]));
						strcat(objtext[objtextlen],"\n");
						objtextlen++;
						while(strcmp(line[i+1].lobjob,"")==0){
							i++;
						}
					}

				}
				else if(strcmp(line[i].op,"RESW")==0 || strcmp(line[i].op,"RESB")==0){
					is_start=1;
					strcpy(objtext[objtextlen],objline);
					int hex=strlen(objlineafter)/2;
					char str[3];
					itoa(hex,str,16);
					str[2]='\0';
					if(strlen(str)>2){
						printf("objlinelen error...");
						return 0;
					}
					if(strlen(str)==1){
						str[1]=str[0];
						str[0]='0';
					}
					strcpy(objlinelen,str);
					strcat(objtext[objtextlen],objlinelen);
					strcat(objtext[objtextlen],objlineafter);
					printf("{%s}(%d)(%d)\n",objtext[objtextlen],strlen(objlineafter),strlen(objtext[objtextlen]));
					strcat(objtext[objtextlen],"\n");
					objtextlen++;
					while(strcmp(line[i+1].lobjob,"")==0){
						i++;
					}
				}

			}

			strcpy(objtext[objtextlen],objline);
			int hex=strlen(objlineafter)/2;
			char str[3];
			itoa(hex,str,16);
			str[2]='\0';
			if(strlen(str)>2){
				printf("objlinelen error...");
				return 0;
			}
			if(strlen(str)==1){
				str[1]=str[0];
				str[0]='0';
			}
			strcpy(objlinelen,str);
			strcat(objtext[objtextlen],objlinelen);
			strcat(objtext[objtextlen],objlineafter);
			printf("{%s}(%d)(%d)\n",objtext[objtextlen],strlen(objlineafter),strlen(objtext[objtextlen]));
			strcat(objtext[objtextlen],"\n");
			objtextlen++;



			
			//M
			Mfrontsum=0;
			int bufM=0;
			for(i=0;i<line_len;i++){
				if(line[i].is_direct==1){
					bufM+=1;
					Mlenth=5;
					char strMlenth[3];
					char sbufM[7];
					itoa(bufM,sbufM,16);

					int lendiff=6-strlen(sbufM);
					if(lendiff>0){
						int sbufMi;
						for(sbufMi=strlen(sbufM)-1;sbufMi>=0;sbufMi--){
							sbufM[sbufMi+lendiff]=sbufM[sbufMi];
						}
						lendiff--;
						while(lendiff>=0){
							sbufM[lendiff]='0';
							lendiff--;
						}
						sbufM[6]='\0';
					}


					strcpy(strMlenth,"05");
					strcpy(objtext[objtextlen],"M");
					strcat(objtext[objtextlen],sbufM);
					strcat(objtext[objtextlen],strMlenth);
					printf("{%s}\n",objtext[objtextlen]);
					strcat(objtext[objtextlen],"\n");
					objtextlen++;
					//printf("Mlen(%06X) pc(%06X)loc(%06X) : %12s %12s(%06X) %12s,%12s obj<%s>           (FMT=%X, ADDR=%X)\n",bufM,locctr[i+1], locctr[i], line[i].symbol, line[i].op,line[i].code, line[i].operand1, line[i].operand2,line[i].lobjob, line[i].fmt, line[i].addressing);
				}
				Mfrontsum+=line[i].objlen;
				bufM=Mfrontsum;
				//if(line[i].is_direct==1)printf("pc(%06X)loc(%06X) : %12s %12s(%06X) %12s,%12s obj<%s>           (FMT=%X, ADDR=%X)\n",locctr[i+1], locctr[i], line[i].symbol, line[i].op,line[i].code, line[i].operand1, line[i].operand2,line[i].lobjob, line[i].fmt, line[i].addressing);
			}
			
			//E
			char endstr[7];
			itoa(endrec,endstr,16);
			int lendiff=6-strlen(endstr);
			if(lendiff>0){
				int endstri;
				for(endstri=strlen(endstr)-1;endstri>=0;endstri--){
					endstr[endstri+lendiff]=endstr[endstri];
				}
				lendiff--;
				while(lendiff>=0){
					endstr[lendiff]='0';
					lendiff--;
				}
				endstr[6]='\0';
			}
			strcpy(objtext[objtextlen],"E");
			strcat(objtext[objtextlen],endstr);
			printf("{%s}",objtext[objtextlen]);
			strcat(objtext[objtextlen],"\n");
			objtextlen++;

			//toupper
			int j;
			for(i=0;i<objtextlen;i++){
				for(j=0;j<strlen(objtext[i]);j++){
					if(objtext[i][j]>=97&&objtext[i][j]<=122){
						objtext[i][j]-=32;
					}
				}
			}

			//printf
			printf("\n");
			for(i=0;i<objtextlen;i++){
				printf("{%s",objtext[i]);
			}

			//write file
			FILE *fp2=fopen(argv[2],"w");
			for(i=0;i<objtextlen;i++){
				fprintf(fp2,objtext[i]);
			}
			fclose(fp2);

		}
		

		
	}
}

