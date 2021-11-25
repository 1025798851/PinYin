#include <io.h>
#include <fcntl.h>
#include <Windows.h>
//#include <stringapiset.h>

#include <iostream>
#include <vector>
#include <string>

#include "PinYin.h"
#include "utils.h"

int main()
{
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

	GetUnihanReadings();

	std::wstring in;
	std::wstring out;
	do
	{
		std::wcout << L"���뺺�֣�";
		std::wcin >> in;

		out.clear();
		if (!in.empty())
		{
			// ת�幦��
			// \x ��\x���16�����ַ���(\x20-\x10FFFF)תΪ�ַ�
			// \d ��\x���10�����ַ���(\d32-\d1114111)תΪ�ַ�
			// \o ��\x���8�����ַ���(\o40-\o4177777)תΪ�ַ�
			// \b ��\x���2�����ַ���(\b100000-\b100001111111111111111)תΪ�ַ�
			// \g ��\g���10�����ַ���(\g0-\g(PinYinSize))תΪ�ַ������ڼ��PinYin.h��Ӧ������ʾ������
			{
				size_t lxi = 0;
				do
				{
					enum class EscapeEnum : char
					{
						none = 0,
						x,
						d,
						o,
						b,
						g,
					};
					EscapeEnum flag = EscapeEnum::none;

					size_t tlxi = in.find(L"\\x", lxi);
					if (tlxi != -1)
						flag = EscapeEnum::x;
					else if ((tlxi = in.find(L"\\d", lxi)) != -1)
						flag = EscapeEnum::d;
					else if ((tlxi = in.find(L"\\o", lxi)) != -1)
						flag = EscapeEnum::o;
					else if ((tlxi = in.find(L"\\b", lxi)) != -1)
						flag = EscapeEnum::b;
					else if ((tlxi = in.find(L"\\g", lxi)) != -1)
						flag = EscapeEnum::g;

					if (flag != EscapeEnum::none)
					{
						lxi = tlxi;
						const size_t lxi2 = lxi + 2;
						wchar_t* EndPtr = NULL;
						const wchar_t* StartPtr = in.c_str() + lxi2;

						switch (flag)
						{
						case EscapeEnum::x:
						case EscapeEnum::d:
						case EscapeEnum::o:
						case EscapeEnum::b:
						{
							const long l = wcstol(StartPtr, &EndPtr, ((flag == EscapeEnum::x) ? 16 : (flag == EscapeEnum::d) ? 10 : (flag == EscapeEnum::o) ? 8 : 2));
							if ((l >= 0x20 || l == 0x09) && l <= 0x10FFFF)	// 0x09��TAB��������, wchar_t/char16_tֻ֧�ֵ�0x10FFFF��UTF32����0x10FFFF�������޷���ʾ��
							{
								std::wstring UTF16;
								UTF32ToUTF16(l, UTF16);

								const size_t count = EndPtr - StartPtr + 2;
								in.replace(lxi, count, UTF16);
							}
							else
								lxi += 2;
							break;
						}

						case EscapeEnum::g:
						{
							long l = wcstol(StartPtr, &EndPtr, 10);
							if (l < PinYinSize)
							{
								l -= 8;	// PinYin.h��ƫ������

								if (l >= 0x00000 && l <= 0x019BF)		// 0x3400 - 0x4DBF
									l += 0x3400;
								else if (l >= 0x019C0 && l <= 0x06BBF)	// 0x4E00 - 0x9FFF
									l += 0x3440;
								else if (l >= 0x06BC0 && l <= 0x06DBF)	// 0xF900 - 0xFAFF
									l -= 0x8D40;
								else if (l >= 0x06DC0 && l <= 0x1149F)	// 0x20000 - 0x2A6DF
									l -= 0x19240;
								else if (l >= 0x114A0 && l <= 0x1598F)	// 0x2A700 - 0x2EBEF
									l -= 0x19260;
								else if (l >= 0x15990 && l <= 0x15BAF)	// 0x2F800 - 0x2FA1F
									l -= 0x19E70;
								else if (l >= 0x15BB0 && l <= 0x16EFF)	// 0x30000 - 0x3134F
									l -= 0x1A450;

								std::wstring UTF16;
								UTF32ToUTF16(l, UTF16);

								const size_t count = EndPtr - StartPtr + 2;
								in.replace(lxi, count, UTF16);
							}
							else
								lxi += 2;
							break;
						}
							
						default:
							break;
						}
						
						continue;
					}

					break;
				} while (true);
			}
			
			for (size_t i = 0; i < in.size(); i++)
			{
				if (i != 0)
					out += L" ";

				size_t j = UTF16ToUTF32((char16_t*)&in[i]);
				if (j > 0xFFFF)
					i++;

				// 0x2E80  - 0x2EFF  CJK Radicals Supplement ���ֲ��� 128��
				// 0x2F00  - 0x2FDF  Kangxi Radicals         �������� 224��
				// 
				// 0x3400  - 0x4DBF  CJK Unified Ideographs Extension A			6592������
				// 0x4E00  - 0x9FFF  CJK Unified Ideographs						20992������
				// 0xF900  - 0xFAFF  CJK Compatibility Ideographs				512������
				// 0x20000 - 0x2A6DF CJK Unified Ideographs Extension B			42720������
				// 0x2A700 - 0x2B73F CJK Unified Ideographs Extension C			4160������
				// 0x2B740 - 0x2B81F CJK Unified Ideographs Extension D			224������
				// 0x2B820 - 0x2CEAF CJK Unified Ideographs Extension E			5776������
				// 0x2CEB0 - 0x2EBEF CJK Unified Ideographs Extension F			7488������
				// 0x2F800 - 0x2FA1F CJK Compatibility Ideographs Supplement	544������
				// 0x30000 - 0x3134F CJK Unified Ideographs Extension G			4944������

				if (j >= 0x3400 && j <= 0x4DBF)			// 0x0000  - 0x19BF
					j -= 0x3400;								   
				else if (j >= 0x4E00 && j <= 0x9FFF)	// 0x19C0  - 0x6BBF
					j -= 0x3440;								   
				else if (j >= 0xF900 && j <= 0xFAFF)	// 0x6BC0  - 0x6DBF
					j -= 0x8D40;								   
				else if (j >= 0x20000 && j <= 0x2A6DF)	// 0x6DC0  - 0x1149F
					j -= 0x19240;
				else if (j >= 0x2A700 && j <= 0x2EBEF)	// 0x114A0 - 0x1598F
					j -= 0x19260;
				else if (j >= 0x2F800 && j <= 0x2FA1F)	// 0x15990 - 0x15BAF
					j -= 0x19E70;
				else if (j >= 0x30000 && j <= 0x3134F)	// 0x15BB0 - 0x16EFF
					j -= 0x1A450;
				else
				{
				PinYinOutSource:
					out += in[i];
					continue;
				}

				for (size_t k = 0; k < 7; k++)
				{
					if (PinYin[j][k][0] == 0)
						goto PinYinOutSource;
					if (k != 0)
						out += L"/";

					out += UTF8ToUTF16(PinYin[j][k]); // �� PinYin[] �����͸�Ϊwchar_t/char16_t���Խ�ʡ�����ת�����㣬�������������ڴ�
				}
				goto PinYinOutSource;
			}
		}
		else
		{
			break;
		}

		SetClipboardText(out.c_str()); // ���õ�ǰ���ֵ����а�

		std::wcout << out << std::endl;
	} while (true);
}