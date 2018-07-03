#include <iostream>  
using namespace std;
//TODO:
// make use of blue count  !!!
class Master
{
    public:
    vector<int> previousDigits;
    vector<int> previousMNIST;
    vector<int> currentDigits;
    vector<int> currentMNIST;
    int failureCount;
    bool failure;
    int shootIndex;
    int blueCount;
    Master()
    {
        failureCount = 0;
        shootIndex = 0;
        for(int i = 0; i<9;i++)
        previousMNIST.push_back(-3);
        for(int i = 0; i<5;i++)
        previousDigits.push_back(-3);
    }
    void clearVecor()
    {
        if(currentDigits.size() == 5)
        {
            previousDigits.clear();
            for(int i = 0; i<5;i++)
            {
                previousDigits.push_back(currentDigits[i]);
            }
        }
        if(currentMNIST.size() == 9)
        {
            previousMNIST.clear();
            for(int i = 0; i<9;i++)
            {
                previousMNIST.push_back(currentMNIST[i]);
            }
        }
        currentDigits.clear();
        currentMNIST.clear();
    }
    
    int whichToShootAuto(ofstream& file)
    {
        bool digitChaged = false;
        bool mnistChaged = false;
        if(currentDigits.size()!=5 || currentMNIST.size()!=9)
        {
            record(file,true,-1);
            currentDigits.clear();
            currentMNIST.clear();
            return -1;
        }
        for(int i = 0;i<5;i++)
        {
            if(previousDigits[i]!=currentDigits[i])
            {
                digitChaged = true;
                break;
            }
        }
        for(int i = 0;i<9;i++)
        {
            if(previousMNIST[i]!=currentMNIST[i])
            {
                mnistChaged = true;
                break;
            }
        }
        if(digitChaged)
        {
            shootIndex = 0;
            failureCount = 0;
            record(file,true,previousDigits[0]);
            clearVecor();
            return  previousDigits[0];
        }
        else if((!digitChaged)&&mnistChaged)
        {
            
            if(shootIndex > 4)
            {
                shootIndex = 0;
                record(file,true,-1);
                clearVecor();
                return -1; 
            }
            failureCount = 0;
            record(file,true,previousDigits[shootIndex]);
            clearVecor();
            shootIndex++;
            return previousDigits[shootIndex - 1];
        }
        else
        {
            record(file,true,-1);
            clearVecor();
            return -1;
        }   
    }
    int whichToShootSemiAuto(ofstream& file,int target)
    {
        bool mnistChaged = false;
        if(currentMNIST.size()!=9)
        {
            record(file,false,-1);
            currentMNIST.clear();
            return -1;
        }
        for(int i = 0;i<9;i++)
        {
            if(previousMNIST[i]!=currentMNIST[i])
            {
                mnistChaged = true;
                break;
            }
        }
        if(mnistChaged)
        {
            failureCount = 0;
            record(file,false,target);
            clearVecor();
            return target;
        }
        else
        {
            record(file,false,-1);
            clearVecor();
            return -1;
        }   
    }
    void record(ofstream& file,bool AUTO,int returnValue)
    {
        file<<"*****************************"<<endl;
        if(AUTO)
        {
            file<<"current digits ";
            for(int i = 0;i<currentDigits.size();i++)
                file<<currentDigits[i]<<" ";
            file<<endl;
            file<<"previous digits "<<previousDigits[0]<<" "
                                    <<previousDigits[1]<<" "
                                    <<previousDigits[2]<<" "
                                    <<previousDigits[3]<<" "
                                    <<previousDigits[4]<<endl;
        }
        file<<"previous MNISTS: ";
        for(int i = 0;i<previousMNIST.size();i++)
            file<<previousMNIST[i]<<" ";
        file<<endl;

        file<<"current MNISTS: ";
        for(int i = 0;i<currentMNIST.size();i++)
            file<<currentMNIST[i]<<" ";
        file<<"return value "<<to_string(returnValue)<<endl;
        file<<"*****************************"<<endl;
    }
    void Fail()
    {
        clearVecor();
        failureCount++;
        return ;
    }
};