#include <iostream>
#include <windows.h>

int main()
{
	//ģ���ڴ��е��ֽ�����
	BYTE MemByte[] = { 0x33, 0xC9, 0x89, 0x0D, 0xB4, 0x67, 0x92, 0x77, 0x89, 0x0D, 0xB8, 0x67, 0x92, 0x77, 0x88, 0x08, 0x38, 0x48, 0x02, 0x74, 0x05, 0xE8, 0x94, 0xFF, 0xFF, 0xFF, 0x33, 0xC0, 0xC3, 0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x83, 0xE4, 0xF8 };

	//������Ϊ��?9 ?? 0? ?? 67
	//�ᴦ��ɣ�F9 FF 0F FF 67 ����ƥ��
	std::string pattern = "?9 ?? 0? ?? 67";
	int index = 0;
	while ((index = pattern.find(' ', index)) >= 0) pattern.erase(index, 1); //ȥ�����������пո�
	size_t len = pattern.length() / 2; //���������볤��
	size_t nFirstMatch = len;  // ����ͷ��??����¼��һ��ƥ���λ�ð��ַ����??�������Ż�����
	BYTE* pMarkCode = new BYTE[len];  // �洢ת������������ֽ�
	BYTE* pWildcard = new BYTE[len];  // �洢�����ַ�����??��?(??=FF��?=F����?=0) ͨ���

	//�����������ַ�����ת�����ֽ�����
	for (size_t i = 0; i < len; i++)
	{
		std::string tmpStr = pattern.substr(i * 2, 2);
		if ("??" == tmpStr) // ��"??"�������ַ�
		{
			tmpStr = "FF";
			pWildcard[i] = 0xFF;
		}
		else  // ����"??"�������ַ�
		{
			if ('?' == tmpStr[0]) // ����ֽ�Ϊ'?'
			{
				tmpStr[0] = 'F';
				pWildcard[i] = (0xF << 4);
			}
			else if ('?' == tmpStr[1]) // �Ұ��ֽ�Ϊ'?'
			{
				tmpStr[1] = 'F';
				pWildcard[i] = 0xF;
			}
			else
			{
				pWildcard[i] = 0x0;
			}
			if (nFirstMatch == len) nFirstMatch = i;
		}

		pMarkCode[i] = strtoul(tmpStr.c_str(), nullptr, 16);
	}

	//�����ڴ棬ƥ���������㷨
	for (size_t m = 0; m < sizeof(MemByte); ++m)
	{
		if (!((MemByte[m] | pWildcard[nFirstMatch]) ^ pMarkCode[nFirstMatch])) //ƥ���ϵ�һ���ֽ�
		{
			size_t offset = m - nFirstMatch; //��¼ƫ����
			for (size_t n = nFirstMatch; n < len; ++n) //ƥ������ֽ�
			{
				if (offset > sizeof(MemByte) - len) break; //�����ڴ淶Χ
				if (pWildcard[n] != 0xFF)  //�����ֽ���"??"��ͨ���������������������Ż�����
					if ((MemByte[offset + n] | pWildcard[n]) ^ pMarkCode[n]) break; //ƥ��ʧ��
				if (n + 1 == len) //ƥ��ɹ�
				{
					printf("%Ix\n", MemByte[m - nFirstMatch]);
				}
			}
		}
	}
	system("pause");
	return 0;
}