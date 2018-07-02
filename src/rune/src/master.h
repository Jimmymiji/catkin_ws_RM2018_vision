#include <iostream>  
using namespace std;

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
    Master()
    {
        failureCount = 0;
        shootIndex = 0;
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
            for(int i = 0; i<5;i++)
            {
                previousMNIST.push_back(currentMNIST[i]);
            }
        }
        currentDigits.clear();
        currentMNIST.clear();
    }
    
    int whichToShoot()
    {
        bool digitChaged = false;
        bool mnistChaged = false;
    
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
            clearVecor();
            return  previousDigits[0];
        }
        else if((!digitChaged)&&mnistChaged)
        {
            shootIndex++;
            if(shootIndex > 5)
            {
                shootIndex = 0;
                clearVecor();
                return -1;
            }
            failureCount = 0;
            clearVecor();
            return previousDigits[shootIndex - 1];
        }
        else
        {
            clearVecor();
            return -1;
        }   
    }

    void Fail()
    {
        clearVecor();
        failureCount++;
        return ;
        
    }
};