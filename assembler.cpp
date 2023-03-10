#include <iostream>
#include <sstream>
#include <string>
#include <fstream> 
#include <stdlib.h>
#include <stdio.h>
#include <map>
using namespace std;

string intToHexString(int n, int sizeOfChar)
{
    string hexString;
    hexString.resize(sizeOfChar, '0');
    for (int i = 0; n != 0; i++)
    {
        if ((n % 16)>=10)hexString[sizeOfChar-i-1] = n % 16 -10 + 'A';
		else hexString[sizeOfChar-i-1] = n % 16 + '0';
		n/= 16;
    }
    return hexString;
}

int main(int argc, char **argv)
{
struct 
	{
    string opeIns[11]=
		{"PSHA","PSHX","PSHF","POPA","POPX","POPF","SHRA","SHLA","RTN","NOP","HLT"};
    string oprBin[11]=
		{"FFF010","FFF011","FFF012","FFF013","FFF014","FFF015","FFF022","FFF023","FFF040","FFF038","FFFFFF"}; 
	} opeInsTable1;

struct 
	{
    string opeIns[31]=
		{
		"LDA","LDX","LDS","LDF","STA","STX","STS","STF","PSH","POP","CLR","SET","ADA","ADX","ADS",
		"ADF","SBA","SBX","SBS","SBF","MUL","DIV","MOD","CMA","CMX","CMS","CMF","JSR","JEQ","JLT","JGE"
		};
    string oprBin[31]=
		{
		"00","01","02","03","04","05","06","07","08","09","0A","0B","10","11","12",
		"13","14","15","16","17","18","19","1A","20","21","22","23","41","48","4A","4B",
		}; 
	int addressModeSupported[42]=    //2: 0~9 ; 1: 2~9; 0:NA
		{2,2,2,2,1,1,1,1,2,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1};
	} opeInsTable2;
	
fstream fin,fout;
fin.open(argv[1], ios::in);
if (!fin.is_open()) 
    { 
        cerr << "FAIL OPEN INPUT FILE" << endl; 
		return -1;
    } 
fout.open(argv[2], ios::out);
if (!fout.is_open()) 
    { 
        cerr << "FAIL OPENOUTPUT FILE" << endl; 
		return -1;
    } 

//*********** 1st go through generation of the symbol table
map<string, int> symbolTable;
int addressCounter = 0; 
string inLine;
while(getline(fin,inLine))
	{

	if (inLine >= "a") // if bigger than “a”, string is lowercase = label: 
		{
			if((symbolTable.find(inLine))==symbolTable.end()) // not found end()
				symbolTable.insert(pair<string,int>(inLine,addressCounter));
			else
				{
				cerr<<"ASSEMBLER ERROR: label defined multiple times."<<endl;
				return -1;
				}
		}
	else //.word or upper alphabet
		addressCounter++;
	}
//	map<string, int>::iterator iter; for(iter=symbolTable.begin();iter!=symbolTable.end();iter++) cout<<iter->first<<"---"<<iter->second<<endl; 
	
//************ 2nd go through: assemble code to bin code (ASCII display) 
fin.close();
fin.open(argv[1], ios::in);
while(getline(fin,inLine))
{
	string outLine;
	outLine = inLine;
//--0) label  
	if (inLine >= "a") continue;
	
//--0) phase word into string array word[4] 
	istringstream istrm(inLine);
	string word[4];
	int numOfWords=0;
	while ( istrm )
		{
		istrm>> word[numOfWords];
		numOfWords++;
		}
	numOfWords--;
	
//--1) analyse: 1 word instruction
	if (numOfWords==1)
	{
		int i; 
		for ( i=0; i<11;i++)
			{
			if (word[0] == opeInsTable1.opeIns[i])
				{
				outLine=opeInsTable1.oprBin[i];
				break;
				}
			}
		if (i<11) 
			{
			//cout << outLine << endl;
			fout << outLine << endl;
			
			continue; //next line 
			}
		else  
			{
			cerr << "ASSEMBLER ERROR: no such instruction" << endl;
			return -1; //next line 
			}
	}
	
//--2) analyse: 2 word instruction
	else if (numOfWords==2)
	{	
		// word[0]== .word data segment
		if (word[0] == ".word")     
			{
			outLine= word[1];
			int addZeroNum =6-word[1].length();
			for (int i=0; i< addZeroNum;i++)
					outLine="0" + outLine;	
			//cout << outLine << endl;
			fout << outLine << endl;	
			continue;
			}
		// word[0]== instruction , CODE segment	
		else                
			{
			//OPT CODE TO BIN 
			int optID;
			for (optID=0; optID<31;optID++)
				{
				if (word[0] == opeInsTable2.opeIns[optID])
					{
					outLine=opeInsTable2.oprBin[optID];
					break;
					}
				}
				
			//ADDRESS MODE 
			outLine= outLine+"2";
			
			//CONVERT ADDRESS VALUE to HEX ASCII		
			if (word[1] >= "a") // label
				{
				word[1]+=":";
				if((symbolTable.find(word[1]))!= (symbolTable.end())) 
				 	{
					outLine=outLine+ intToHexString (symbolTable.find(word[1])->second,3);
					//cout << outLine << endl;
					fout << outLine << endl;	
					continue;					
					}
				else
					{
					cerr<<"ASSEMBLER ERROR: undefined label."<<endl;
					return -1;
					}	
				}
			else if (word[1] >= "0")  //01234567890ABCDEF not $
				{
				outLine=outLine+intToHexString(atoi(word[1].c_str()),3);	
				//cout << outLine << endl;
				fout << outLine << endl;	
				continue; 
				}
			else if ((word[1] >= "$0") && (word[1] <= "$FFF"))
				{
				if (word[1].length()==2)
					outLine=outLine+"00"+word[1][1];	
				else if (word[1].length()==3)
					outLine=outLine+"0"+word[1][1]+word[1][2];	
				else if (word[1].length()==4)
					outLine=outLine+word[1][1]+word[1][2]+word[1][3];	
				//cout << outLine << endl;
				fout << outLine << endl;	
				continue; 
				}
			else
				{
				cerr<<"ASSEMBLER ERROR: undefined label ."<<endl;
				return -1;
				}	
			}
		}			
// --3) analyse: 3  word instruction
	else if (numOfWords==3)
	{
		//OPT CODE TO BIN
		int optID;
		for (optID=0; optID<31;optID++)
			{
			if (word[0] == opeInsTable2.opeIns[optID])
				{
				outLine=opeInsTable2.oprBin[optID];
				break;
				}
			}
				
		//ADDRESS MODE 
		if (word[1]=="#") 
			{
			if (opeInsTable2.addressModeSupported[optID]==1)
				{
				cerr << "ASSEMBLER ERROR: instruction using unsupported addressing mode."<<endl;
				//cerr << "ILLEGAL: STA does not support immediate addressing" << endl;
				return -1;
				}
			else outLine= outLine+"0";
			}
		else if (word[1]=="!") outLine= outLine+"3";
		else if (word[1]=="+") outLine= outLine+"4";
		else if (word[1]=="*") outLine= outLine+"6";
		else if (word[1]=="&") outLine= outLine+"8";
			
		//CONVERT ADDRESS VALUE 		
		if (word[2] >= "a") // label
			{
			word[2]+=":";
			if((symbolTable.find(word[2]))!= (symbolTable.end())) 
				{
				outLine=outLine+ intToHexString (symbolTable.find(word[2])->second,3);
				// cout << symbolTable.find(word[2])->second << endl;
				fout << outLine << endl;	
				continue;					
				}
			else
				{
				cerr<<"ASSEMBLER ERROR: undefined label ."<<endl;
				return -1;
				}	
			}
		else if (word[2] >= "0")  //01234567890ABCDEF not $
			{
			outLine=outLine+intToHexString(atoi(word[2].c_str()),3);	
			//cout << outLine << endl;
			fout << outLine << endl;	
			continue; 
			}
		else if ((word[2] >= "$0") && (word[2] <= "$FFF"))
			{
			if (word[2].length()==2)
				outLine=outLine+"00"+word[2][1];	
			else if (word[2].length()==3)
				outLine=outLine+"0"+word[2][1]+word[2][2];	
			else if (word[2].length()==4)
				outLine=outLine+word[2][1]+word[2][2]+word[2][3];	
			//cout << outLine << endl;
			fout << outLine << endl;	
			continue; 
			}
		else
			{
			cerr<<"ASSEMBLER ERROR: undefined label ."<<endl;
			return -1;
			}	
		}	
// --4) analyse: 4  word instruction	
	else if (numOfWords==4)
		{
		//OPT CODE TO BIN 
		int optID;
		for (optID=0; optID<31;optID++)
			{
			if (word[0] == opeInsTable2.opeIns[optID])
				{
				outLine=opeInsTable2.oprBin[optID];
				break;
				}
			}
				
		//ADDRESS MODE 
		if (word[1]=="#"&&word[2]=="!") 
			{
			if (opeInsTable2.addressModeSupported[optID]==1)
				{
				cerr << "ASSEMBLER ERROR: instruction using unsupported addressing mode."<<endl;
				//cerr << "ILLEGAL: JSR does not support frame immediate addressing" << endl;
				return -1;
				}
			else outLine= outLine+"1";
			}
		else if (word[1]=="+"&&word[2]=="!") outLine= outLine+"5";
		else if (word[1]=="*"&&word[2]=="!") outLine= outLine+"7";
		else if (word[1]=="&"&&word[2]=="!") outLine= outLine+"9";
			
		//CONVERT ADDRESS VALUE 
		if (word[3] >= "a") // label
			{
			word[3]+=":";
			if((symbolTable.find(word[3]))!= (symbolTable.end())) 
				{
				outLine=outLine+ intToHexString (symbolTable.find(word[3])->second,3);
				// cout << symbolTable.find(word[3])->second << endl;
				fout << outLine << endl;	
				continue;					
				}
			else
				{
				cerr<<"ASSEMBLER ERROR: undefined label ."<<endl;
				return -1;
				}	
			}		
		if (word[3] >= "0")  //01234567890ABCDEF not $
			{
			outLine=outLine+intToHexString(atoi(word[3].c_str()),3);	
			//cout << outLine << endl;
			fout << outLine << endl;	
			continue; 
			}
		else if ((word[3] >= "$0") && (word[3] <= "$FFF"))
			{
			if (word[3].length()==2)	outLine=outLine+"00"+word[3][1];	
			else if (word[3].length()==3)	outLine=outLine+"0"+word[3][1]+word[3][2];	
			else if (word[3].length()==4)	outLine=outLine+word[3][1]+word[3][2]+word[3][3];	
			//cout << outLine << endl;
			fout << outLine << endl;	
			continue; 
			}
		else
			{
			cerr<<"ASSEMBLER ERROR: undefined label ."<<endl;
			return -1;
			}		 
			
		}		
}
	
fin.close();
fout.close();
return 0;
}