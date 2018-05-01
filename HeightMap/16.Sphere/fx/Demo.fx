//
//
// Demo.fx : �⺻ ���̴� �ҽ�.
//
// 2003.11.4. Kihong Kim / mad_dog@hanmail.net
// 2010.12.01. Update. (DX11.Jun.2010)
// 2016.12.27. Update. (DX11. Windows SDK 8.1)
//


// ��� ����
cbuffer ConstBuffer //: register(b0)
{
    matrix mTM;     //���� ���. 
    matrix mView;   //�� ��ȯ ���. 
    matrix mProj;   //���� ��ȯ ���. 
};

 
//VS ��� ����ü.
struct VSOutput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
};




////////////////////////////////////////////////////////////////////////////// 
//
// Vertex Shader Main : ���� ���̴� ���� �Լ�.
//
////////////////////////////////////////////////////////////////////////////// 

VSOutput VS_Main(
				  float4 pos : POSITION,    //[�Է�] ������ǥ. Vertex Position (Model Space, 3D)
				  float4 col : COLOR0       //[�Է�] ������ Vertex Color : "Diffuse"
				)
{
	//* �Ʒ��� �׽�Ʈ�� �����ϱ� ����  
    //* VS �� ��� ���۰� �����Ǿ� �־�� �մϴ�.    
    pos.w = 1;

	//���� ��ȯ.(World Transform) 
    pos = mul(pos, mTM);        //pos = pos * mTM
	
    //�þ�-�� ��ȯ (View Transform)
    pos = mul(pos, mView);

    //���� ���� ��ȯ (Projection Transform)
    pos = mul(pos, mProj);


    //���� ���.
    VSOutput o = (VSOutput) 0;
    o.pos = pos; 
    o.col = col;

    return o;
}





////////////////////////////////////////////////////////////////////////////// 
//
// Pixel Shader Main : �ȼ� ���̴� ���� �Լ�.
//
////////////////////////////////////////////////////////////////////////////// 

float4 PS_Main(
				float4 pos : SV_POSITION,   //[�Է�] (������) �ȼ��� ��ǥ. (Screen, 2D)
				float4 col : COLOR0         //[�Է�] (������) �ȼ��� ����. (Pixel Color : "Diffuse")
				) : SV_TARGET               //[���] ����.(�ʼ�), "����Ÿ��" ���� ����մϴ�.
{
	//Ư���� ��ȯ ���� �Էµ� ������ �״�� ����մϴ�.
    //...
	 
    return col;
}



/**************** end of file "Demo.fx" ***********************/
