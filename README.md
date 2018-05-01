# DX11

************** 현재 작업 방향 ******************
  Terrain관련 기능을 구현한후
  TrialWorld로의 포팅을 목적으로함

************** 작업 Archive *******************

2018-05-02 12:08 AM
작업내용

  컴파일 상태
    
    컴파일 시도해보지 않음.

  HeightMap.h
    
    구조체
     - HeightTerrain
    
    함수
     - bool TerrainCreate(const TCHAR* rawFile)
       bool TerrainUpdate(float dTime)
       bool TerrainDraw(float dTime)
       void TerrainRelease()
    
    변수
    - extern HeightTerrain* g_pHTerrain
    
  HeightMap.cpp
     
     지역 구조체
      - VTXTERRAIN
     
     지역변수
      - ID3D11Buffer* g_pVB
      - ID3D11InputLayout* g_pInputLayout
      - VTXTERRAIN* g_pVtxData
      - int* g_pIndices
      - DWORD g_vtxCnt
      - UCHAR* g_pHeight
      - UINT g_Width
      - UINT g_Length
     
     지역함수
      - Initialize(const TCHAR* rawFile)
            LoadHeightMap(rawFile)추가
            CreateVertex()추가
            CreateIndex()추가
           
      - LoadHeightMap(const TCHAR* rawFile)
            ifstream을 통한 raw파일 데이터 읽기
            g_VtxCnt에 데이터 갯수 저장, g_pHeight에 높이 정보를 위한 메모리할당 및 높이 정보 읽어오기
            g_Width, g_Length를 구함.(현재 raw파일의 이미지는 정사각형 형태라 가정[g_Width == g_Length]
      - CreateVertex()
            g_VtxCnt만큼 버텍스 데이터 생성
            pivot을 중심으로 하기위해 halfWidth와 halfLength사용
      - CreateIndex()
            메모리사용 절감을 위해 Index list를 만듦
            인덱스 개수는 (Width * 2) * (Length -1) + (Length - 2)
            *****자세한 구현 원리는 Ref파일의 "높이맵 코딩 및 인덱싱, 버텍싱"참고*****
     
     글로벌 함수
      - TerrainCreate(const TCHAR* rawFile)
            테리언 구조체를 생성
            테리언의 Transformation을 설정(pos,  scale, rot)
            버택스버퍼, 인풋레이아웃 생성
            
      - TerrainUpdate(float dTime)
            빈 함수 선언
      - TerrainDraw(float dTime)
            빈 함수 선언
      - TerrainRelease()
            동적할당 및 Create된 함수 Release 및 delete
