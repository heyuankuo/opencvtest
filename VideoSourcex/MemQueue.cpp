#include "stdafx.h"
#include "memqueue.h"
//#using <mscorlib.dll>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

CMemQueue::CMemQueue() 
{ 
	int i = 0; int j = 0;
	for(i=0;i <= MAX_FEEDS_NUM;i++)
	{
		m_pQueFeedNumDataPointers[i]=NULL;
		m_pQueFeedNumHandles[i]=NULL;
		hFillMutex[i].Event1 = NULL;
		hFillMutex[i].Event2 = NULL;
	}
}

CMemQueue::~CMemQueue(void)
{
	for(int i=0; i<= MAX_FEEDS_NUM;i++)
	{
		if(m_pQueFeedNumDataPointers[i] || m_pQueFeedNumHandles[i] )
			CloseQue(i);

		m_pQueFeedNumDataPointers[i]=NULL;
		m_pQueFeedNumHandles[i]=NULL;
	}
	 
}
  
QRESULT	CMemQueue::OpenEvents(int nFeedNum)
{
	HANDLE tmpHandle = NULL;
	if (hFillMutex[nFeedNum].Event1 != 0)
		return ERR_OK;
	//Create memory map name
	char MemMutexName1[100];
	char MemMutexName2[100];
	//  manual reset; non-signaled
	//TODO ¶ÑÕ»Òç³ö´íÎó£¡by Andy
	sprintf(MemMutexName1,"Mvs_Mutex1__FEED#_%d", nFeedNum);
	sprintf(MemMutexName2,"Mvs_Mutex2__FEED#_%d", nFeedNum);

	tmpHandle = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, (LPCSTR)MemMutexName1); 
	DWORD iii1 = GetLastError(); 
	if (tmpHandle == NULL)
	{
		tmpHandle = CreateMutexA(NULL, FALSE, (LPCSTR)MemMutexName1); 
		iii1 = GetLastError(); 
		if (iii1 != 0)
			iii1++;
	}
	hFillMutex[nFeedNum].Event1  = (long)tmpHandle;
	tmpHandle = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, (LPCSTR)MemMutexName2); 
	if (tmpHandle == NULL)
	{
		tmpHandle = CreateMutexA(NULL, FALSE, (LPCSTR)MemMutexName2); 
	}
	hFillMutex[nFeedNum].Event2  = (long)tmpHandle;

	if ((hFillMutex[nFeedNum].Event1 == 0) || (hFillMutex[nFeedNum].Event2 == 0))
	{
		return	ERR_CREATE_QUE;
	}
	return ERR_OK;
}


//Parameters...
//---------------
//int	nFeedNum	-The shared memory name as the feed number
//bool	bCreate		-Create new memory map or just open existing
//Return value...
//---------------
//Bool				-true if get pointer to memory map otherwise false.
QRESULT	CMemQueue::OpenQue(int nFeedNum,FrameHeader*  pFrameHeader)
{
	if(nFeedNum>MAX_FEEDS_NUM)
		return	ERR_FEED_NUM;

	if(m_pQueFeedNumHandles[nFeedNum])
	{

		//Get temp pos
		 BYTE*	pMemData=m_pQueFeedNumDataPointers[nFeedNum];
		//Add que ref count
		 GlobalHeader	gH;
		 memcpy(&gH, pMemData,sizeof(GlobalHeader));
		 gH.nScCounter++;
		 memcpy(pMemData,&gH,sizeof(GlobalHeader));
		 pMemData+=sizeof(GlobalHeader);

		 //Read frame header
		 //*** what if this position is in writing mode now???
		  memcpy(pFrameHeader,pMemData,sizeof(FrameHeader));
		return ERR_OK;
	}
	//Create memory map name
	char MemMapName[100]="Mvs_Mem_Position_XYZRRTY178232__FEED#_";
	char chFeedName[50];
	_ltoa(nFeedNum,chFeedName,10);
	strcat(MemMapName,chFeedName);
  

	QRESULT iResult;
	if ((iResult = OpenEvents(nFeedNum)) != ERR_OK)
		return iResult;

	//Open Mem map
	DWORD err;
	m_pQueFeedNumHandles[nFeedNum]=OpenFileMappingA(FILE_MAP_ALL_ACCESS,TRUE,MemMapName);
	if (m_pQueFeedNumHandles[nFeedNum] == NULL)
	{
		err = GetLastError();
	}


	if(m_pQueFeedNumHandles[nFeedNum])
	{
		 m_pQueFeedNumDataPointers[nFeedNum] =(BYTE*)MapViewOfFile( m_pQueFeedNumHandles[nFeedNum], 
																FILE_MAP_ALL_ACCESS,0, 0, 0);
		 if(m_pQueFeedNumDataPointers[nFeedNum])
		 {
			 //Get temp pos
			 BYTE*	pMemData=m_pQueFeedNumDataPointers[nFeedNum];
			//Add que ref count
			 GlobalHeader	gH;
			 memcpy(&gH, pMemData,sizeof(GlobalHeader));
			 gH.nScCounter++;
			 memcpy(pMemData,&gH,sizeof(GlobalHeader));
			 pMemData+=sizeof(GlobalHeader);

			 //Read frame header
			 //*** what if this position is in writing mode now???
			  memcpy(pFrameHeader,pMemData,sizeof(FrameHeader));
		 }
		 else
		 {
			CloseQue(nFeedNum);
			return	ERR_OPEN_QUE;
		 }
	}
	else
	{
		return ERR_OPEN_QUE;
	}
 
	return ERR_OK;
}
 
QRESULT	CMemQueue::CreateQue(int nFeedNum,FrameHeader  stFrameHeader, long	lImgSize)
{
	if(nFeedNum>MAX_FEEDS_NUM)
		return	ERR_FEED_NUM;

	QRESULT iResult;
	if ((iResult = OpenEvents(nFeedNum)) != ERR_OK)
		return iResult;


	lImgSize = stFrameHeader.Bpp*stFrameHeader.nHeight*stFrameHeader.nWidth;

	if (lImgSize<5800000)
		lImgSize = 5800000;  // 21.03.06  Default Size 

	//Create memory map name
	char MemMapName[100]="Mvs_Mem_Position_XYZRRTY178232__FEED#_";
	char chFeedName[50];
	_ltoa(nFeedNum,chFeedName,10);
	strcat(MemMapName,chFeedName);

	if(m_pQueFeedNumHandles[nFeedNum] )
		return ERR_ALREADY_EXISTS ; 
	 
	long	lMapSize=	sizeof(GlobalHeader)+3*((sizeof(FrameHeader)+lImgSize));
	
	m_pQueFeedNumHandles[nFeedNum]=::CreateFileMappingA((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE,0,lMapSize,MemMapName);
		
	if(m_pQueFeedNumHandles[nFeedNum])
	{
		m_pQueFeedNumDataPointers[nFeedNum] =(BYTE*)MapViewOfFile( m_pQueFeedNumHandles[nFeedNum], 
															FILE_MAP_ALL_ACCESS,0, 0, 0);
		if(m_pQueFeedNumDataPointers[nFeedNum])
		{
			//Get temp pos
			BYTE*	pMemData=m_pQueFeedNumDataPointers[nFeedNum];
			//Init global header.
			GlobalHeader	gH;
			gH.nScCounter=1;
			gH.nReadBuffer=0;
			gH.lImgSize=lImgSize;
			gH.lQueueSize = lMapSize;
			memcpy(pMemData,&gH,sizeof(GlobalHeader));

			pMemData+=sizeof(GlobalHeader);
			//Write First frame header to the first position.
			memcpy(pMemData,&stFrameHeader,sizeof(FrameHeader));

		}
		else
		{
			CloseQue(nFeedNum);
			return	ERR_CREATE_QUE;
		}
	}
	else
	{
		CloseQue(nFeedNum);
		return ERR_CREATE_QUE;
	}
	return	ERR_OK;
}
 
QRESULT	CMemQueue::CloseQue(int nFeedNum)
{
	if(nFeedNum>MAX_FEEDS_NUM)
		return	ERR_FEED_NUM;

	//for (int j = 0; j < MAX_BUFFERS; j++)
	//{
		//if (m_DisplayFrameEvents[nFeedNum]->Event[j] != NULL)
		//{
		//	CloseHandle(m_DisplayFrameEvents[nFeedNum]->Event[j]);
		//	m_DisplayFrameEvents[nFeedNum]->Event[j] = NULL;
		//}
		if (hFillMutex[nFeedNum].Event1  != NULL)
		{
			CloseHandle((HANDLE)hFillMutex[nFeedNum].Event1 );
			hFillMutex[nFeedNum].Event1  = NULL;
		}
		if (hFillMutex[nFeedNum].Event2  != NULL)
		{
			CloseHandle((HANDLE)hFillMutex[nFeedNum].Event2 );
			hFillMutex[nFeedNum].Event2  = NULL;
		}
//	}

	if(m_pQueFeedNumHandles[nFeedNum])
	{
		 if(m_pQueFeedNumDataPointers[nFeedNum])
		 {
	
			BYTE*	pMemData=m_pQueFeedNumDataPointers[nFeedNum];
			//Get global header
			GlobalHeader*	pGH=(GlobalHeader*)pMemData;
			 
			//Release counter
			pGH->nScCounter--;
			 
			//Close Handels
			CloseHandle(m_pQueFeedNumHandles[nFeedNum]);
			UnmapViewOfFile(m_pQueFeedNumDataPointers[nFeedNum]);
 
		 }
		 else
			 CloseHandle(m_pQueFeedNumHandles[nFeedNum]);

		 m_pQueFeedNumHandles[nFeedNum]=NULL;
		 m_pQueFeedNumDataPointers[nFeedNum]=NULL;

		return ERR_OK;
	}
	else
		return	ERR_CLOSING;
}

QRESULT		CMemQueue::GetFrame(unsigned char* pFrame,int nFeedNum,FrameHeader* pFrameHeader,bool bBlocking)
{


	if(nFeedNum>MAX_FEEDS_NUM)
		return	ERR_FEED_NUM;

	if(!(m_pQueFeedNumHandles[nFeedNum] && m_pQueFeedNumDataPointers[nFeedNum]))
		return	ERR_OPEN_QUE;
	 
	BYTE*	pMemMapData=m_pQueFeedNumDataPointers[nFeedNum];
	//Get global header
	
	GlobalHeader*	pGH=(GlobalHeader*)pMemMapData;
	int				nCurrReadPos = pGH->nReadBuffer;
	pMemMapData+=sizeof(GlobalHeader);
	
	//Read frame header
	long	lImgSize = pGH->lImgSize;
	long	lnewPosize = nCurrReadPos * (sizeof(FrameHeader)+lImgSize);
	pMemMapData+=lnewPosize;
	
	long CurrFrameNum=pFrameHeader->FrameNumber;
	//Set temp frame header
	FrameHeader* 	pQueFrameHeader=(FrameHeader*)pMemMapData;
	//memcpy(&pTempFrameHeader,pMemMapData,sizeof(FrameHeader));
	
//	if( (pQueFrameHeader->nWidth *  pQueFrameHeader->nHeight* pQueFrameHeader->Bpp) != lImgSize )
//		return	ERR_MEMORY_SIZE;


	if(bBlocking)
	{
		while((pQueFrameHeader->FrameNumber<=CurrFrameNum)&& (nCurrReadPos==pGH->nReadBuffer))
		{
			Sleep(1);
		} 

		
	}
	DWORD dwResult = WAIT_OBJECT_0;
	//if (nFeedNum < 60)
	//{
	//	if (nCurrReadPos == 0)
	//		dwResult = WaitForSingleObject((HANDLE)hFillMutex[nFeedNum].Event1 , 10);
	//	else
	//		dwResult = WaitForSingleObject((HANDLE)hFillMutex[nFeedNum].Event2 , 10);
	//}

	if (dwResult == WAIT_OBJECT_0) // Got the Semaphor (Mutex)
	{
		memcpy(pFrameHeader, pMemMapData, sizeof(FrameHeader));
		if(pFrameHeader->FrameNumber<0)
		{
			if (nFeedNum < 60)
			{
				if (nCurrReadPos == 0) 
					ReleaseMutex ((HANDLE)hFillMutex[nFeedNum].Event1);
				else
					ReleaseMutex ((HANDLE)hFillMutex[nFeedNum].Event2);
			}
			return ERR_OPEN_QUE;
		}
		
		pMemMapData+=sizeof(FrameHeader);
		//Copy frame buffer
		lImgSize = pFrameHeader->uLsize ;
		if (lImgSize > pGH->lImgSize)
			lImgSize = pGH->lImgSize; // Patch till we fix the size

		memcpy(pFrame,pMemMapData,lImgSize);
		if (nFeedNum < 60)
		{
			if (nCurrReadPos == 0) 
				ReleaseMutex ((HANDLE)hFillMutex[nFeedNum].Event1);
			else
				ReleaseMutex ((HANDLE)hFillMutex[nFeedNum].Event2);
		}
		return ERR_OK;
	}
	return ERR_OPEN_QUE;
} 

QRESULT		CMemQueue::SetFrame(unsigned char* pFrame,int nFeedNum,FrameHeader* pFrameHeader)
{
	if(nFeedNum>MAX_FEEDS_NUM)
		return	ERR_FEED_NUM;

	if(!(m_pQueFeedNumHandles[nFeedNum] && m_pQueFeedNumDataPointers[nFeedNum]))
		return	ERR_OPEN_QUE;
	  
	BYTE*	pMemMapData = m_pQueFeedNumDataPointers[nFeedNum];

	//Read global buffer.
	GlobalHeader*	pGH=(GlobalHeader*)pMemMapData;
	 	
	//Pointer position 
	//*** what if Frame header change (like size and so...)
	int		nCurrWritePos = (pGH->nReadBuffer + 1) % 3;
	long	lImgSize=pGH->lImgSize;
	long	lPos=  nCurrWritePos * (sizeof(FrameHeader)+lImgSize);
	 
//	if( (pFrameHeader->Bpp*pFrameHeader->nWidth*pFrameHeader->nHeight) != lImgSize)
//		return	ERR_MEMORY_SIZE;

	DWORD dwResult = WAIT_OBJECT_0;
	//if (nFeedNum < 60)
	//{
	//	if (nCurrWritePos == 0)
	//		dwResult = WaitForSingleObject((HANDLE)hFillMutex[nFeedNum].Event1, 10);
	//	else
	//		dwResult = WaitForSingleObject((HANDLE)hFillMutex[nFeedNum].Event2, 10);
	//}
	if (dwResult == WAIT_OBJECT_0) // Got the Semaphor (Mutex)
	{
		//Write frame header
		pMemMapData+=(sizeof(GlobalHeader)+lPos);
		memcpy(pMemMapData,pFrameHeader,sizeof(FrameHeader));

		//Write image
		pMemMapData+=sizeof(FrameHeader);
		lImgSize = pFrameHeader->uLsize ;
		memcpy(pMemMapData,pFrame,lImgSize);
		pGH->nReadBuffer = nCurrWritePos;
		if (nFeedNum < 60)
		{
			if (nCurrWritePos == 0) 
			{
				ReleaseMutex ((HANDLE)hFillMutex[nFeedNum].Event1);
			}
			else
			{
				ReleaseMutex ((HANDLE)hFillMutex[nFeedNum].Event2);
			}
		}
	}
	else
	{
		int i = 9;
	}

	return  ERR_OK;
}






