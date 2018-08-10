#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <stdint.h>
using namespace std;
#endif /* __PROGTEST__ */

class CFile
{
public:
  CFile (void);
  CFile (const CFile&);  
  ~CFile (void);
  CFile & operator=(const CFile&);
  bool Seek (uint32_t offset);
  uint32_t Read (uint8_t * dst, uint32_t bytes);
  uint32_t Write (const uint8_t * src, uint32_t bytes);
  void Truncate (void);
  uint32_t FileSize (void) const;
  void AddVersion (void);
  bool UndoVersion (void);
  void PrintCurrent(void);
  void PrintVersions(void);
private:
    uint8_t *arr;
    uint32_t posInd;
    uint32_t lastInd;
    CFile * versions;
    size_t size;
    void Lenghten(uint32_t bytes);
    void CopyContent(const CFile&);
    void CopyVersions(const CFile&);
};

CFile::CFile (void){
    versions = NULL;
    size = 0;
    posInd = 0;
    lastInd = 0;
}

CFile::CFile (const CFile& src){
   // cout << "kopirujici konstruktor" << endl;
    CopyVersions(src);    
}

CFile::~CFile (void){
    //cout << "Destruktor" << endl;
    if(size > 0)
    delete [] arr;
    
    if(versions != NULL)
    delete versions;      
}

CFile & CFile::operator=(const CFile& src){ 
    if ( &src == this ) {
       cout << "a = a" << endl;
        return *this;
    }
    
    if(arr != NULL){
        delete [] arr;
       // cout << "pole smazano pri operatoru =" << endl;
    }
    
    
    if(versions != NULL){
        delete versions; 
        //cout << "verze smazany pri operatoru =" << endl;
    }
     
    CopyVersions(src);
    //PrintVersions();
    
    return *this;
}

void CFile::Truncate (void){
    lastInd = posInd;

}

void CFile::CopyContent(const CFile& src){
    lastInd = src.lastInd;
    size = lastInd;
    posInd = src.posInd;
    arr = new uint8_t[lastInd];
    
    memcpy(arr,src.arr,lastInd * sizeof(uint8_t)); 
}

void CFile::CopyVersions(const CFile& src){
    CopyContent(src);
    CFile * tmp = src.versions;
    CFile * copy = new CFile();
    versions = copy;
    
    while(tmp != NULL){
        copy->CopyContent(*tmp);
        
        if(tmp->versions != NULL){
            CFile* nextCopy = new CFile();
            copy->versions = nextCopy;
        }else
            copy->versions = NULL;
            
        tmp = tmp->versions;
        copy = copy->versions;
    }

}

void CFile::PrintCurrent(void){
    cout << "Vypis obsahu souboru:" <<endl;
    for(size_t i = 0; i < lastInd; i++)
        cout << unsigned(arr[i]) << " - ";
    cout << endl;
}  

void CFile::PrintVersions(void){
    cout << endl << "Vypis vsech zaloh: " << endl;
    CFile * tmp = this->versions;
    while(tmp != NULL){
        tmp->PrintCurrent();
        tmp = tmp-> versions;
    }   
    
}

void CFile::Lenghten(uint32_t bytes){
            //cout << "realokuji, pozice pointeru je " << unsigned(posInd) << endl;
            uint8_t* tmp;
            tmp = new uint8_t[size+size/2 + bytes];
            memcpy(tmp,arr,size * sizeof(uint8_t));
            if (tmp != NULL) {
                if(size != 0)
                    delete [] arr;
                arr = tmp;
                //cout << "realokovano" << endl;
            }
           size = size + size/2 + bytes;
           //cout << "Nove naalokovane pole je velike " << size << endl;
}
  
uint32_t CFile::FileSize (void) const{
    //cout << "velikost: " << lastInd <<endl;
    return lastInd;
}

bool CFile::Seek(uint32_t offset){
    
    if(offset < 0 || offset > lastInd)
        return false;
    
    posInd = offset;
    return true;
}

uint32_t CFile::Write (const uint8_t * src, uint32_t bytes){
    size_t i;
    
    if (src == NULL)
        return 0;
    
    if(posInd + bytes > size){
            Lenghten(bytes);
    }
    for (i = 0; i < bytes; i ++){
        //cout << "zapisuji 1 bajt: " << unsigned(src[i]) << endl;
        
        arr[posInd] = src[i];
        
        
        if(posInd == lastInd){
            lastInd++;
        }
        
        posInd++;
        //cout << "1 bajt zapsan, pointer posunut" << endl;
    }
    return i;

}

uint32_t CFile::Read(uint8_t * dst, uint32_t bytes){
    size_t i;
    
    if(posInd + bytes > lastInd) //chceme-li cist vice bajtu, nez je v souboru, precteme pouze do konce souboru
        bytes = lastInd - posInd;
    
    for (i = 0; i < bytes; i ++){
        //cout << "ctu 1 bajt: " << dec << unsigned(*pos) << endl;
        dst[i] = arr[posInd]; //*pos;
        posInd++;
        //cout << "1 bajt precten, pointer posunut" << endl;
    }
    return i;
}

void CFile::AddVersion (void){
    CFile * latest = new CFile();
    latest->versions = versions;
    
    (*latest).CopyContent(*this);
    
    versions = latest;
    
}

bool CFile::UndoVersion (void){
        
    if (versions == NULL)
        return false;
    
    CFile * delThis = versions;
    versions = delThis->versions;
    delThis->versions = NULL;
    
    delete [] arr;
    CopyContent(*delThis);
    
    delete delThis;
    return true;
}

#ifndef __PROGTEST__
bool writeTest (CFile & x, const initializer_list < uint8_t > &data, uint32_t wrLen){
  uint8_t tmp[100];
  uint32_t idx = 0;
  

for (auto v:data)
    tmp[idx++] = v;
  return x.Write (tmp, idx) == wrLen;
}

bool readTest (CFile & x, const initializer_list < uint8_t > &data, uint32_t rdLen){
  uint8_t tmp[100];
  uint32_t idx = 0;

  if (x.Read (tmp, rdLen) != data.size ()){
    //  cout << "spatna velikost cteneho pole" << endl;
    return false;
    }
for (auto v:data)
    if (tmp[idx++] != v)
      return false;
  return true;
}

int main (void){
 /* CFile f0;

  assert (writeTest (f0, {10, 20, 30}, 3));
 // f0.PrintCurrent();
  assert (f0.FileSize () == 3);
  assert (writeTest (f0,{60, 70, 80}, 3));
  //f0.PrintCurrent();
  assert (f0.FileSize () == 6);
  assert (f0.Seek (2));
  assert (writeTest (f0,{5, 4}, 2));
  //f0.PrintCurrent();
  assert (f0.FileSize () == 6);
  assert (f0.Seek (1));
  assert (readTest (f0,{ 20, 5, 4, 70, 80}, 7));
  assert (f0.Seek (3));
  f0.AddVersion ();
  assert (f0.Seek (6));
  assert (writeTest (f0, {100, 101, 102, 103}, 4));
  f0.AddVersion ();
  assert (f0.Seek (5));
  //f0.PrintVersions();
  CFile f1 (f0);
  assert (writeTest (f1,{200, 210, 220}, 3));
  assert (f1.Seek (0));
  assert (readTest (f1,{10, 20, 5, 4, 70, 200, 210, 220, 102, 103}, 20));
 
  f1 = f0;
  
  f0.PrintVersions();
  f1.PrintVersions();
  */
  
  return 0;
  
}
#endif /* __PROGTEST__ */