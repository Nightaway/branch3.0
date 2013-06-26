#include "../Macro.h"

#include "View.h"

USING_NS_DRAGON;

std::string ViewParser::parse(char endCh)
{
	char ch; 
	std::string Out;
	while (!s.isEOF() && (ch = (char)s.getChar()))
	{
			// ������ʼ
			if (ch == '$') 
			{
				std::string varible;
				ch = s.getChar();
				// ����${name}��ʽ����
				if (ch == '{')
				{
					ch = s.getChar();
					while (ch != '}') 
					{
						varible += ch;
						ch = s.getChar();
					}
					Out += viewBag[varible];
				}
				// ����if ���
				else if (ch == '[')
				{
					std::string varible_if;
					std::string varible_c;
			
					ch = s.getChar();
					if (ch == '$')
					{
						ch = s.getChar();
						while (ch != ' ' && ch != '	') 
						{
							varible_if += ch;
							ch = s.getChar();
						}

						if (viewBag[varible_if].empty())	
						{
							Out += "";
						}  else 	{

							while (	(ch = s.getChar()) != ']')
							{
								if (ch == '$')
								{
										std::string varible;
										ch = s.getChar();
										// ����${name}��ʽ����
										if (ch == '{')
										{
											ch = s.getChar();
											while (ch != '}') 
											{
												varible += ch;
												ch = s.getChar();
											}
											Out += viewBag[varible];
										} else {
													std::string varible;
													 /*
																��������������ĸ��ͷ
													*/
													if (::isalpha(ch)) 
													{
														varible += ch;
														ch = s.getChar();

														/*
																������֮���������ĸ���ֻ��»���
														*/
														while (::isalnum(ch)) 
														{
															varible += ch;
															ch = s.getChar();
														}
														s.ungetChar(ch);

														Out += viewBag[varible];
													}
										}
								} else {
										Out += ch;
								}
							}
						}

						while (ch != ']')
							ch = s.getChar();
					}
				}	else	{
					 /*
								��������������ĸ��ͷ
					*/
					if (::isalpha(ch)) 
					{
						varible += ch;
						ch = s.getChar();

						/*
								������֮���������ĸ���ֻ��»���
						*/
						while (::isalnum(ch)) 
						{
							varible += ch;
							ch = s.getChar();
						}
						s.ungetChar(ch);

						Out += viewBag[varible];
					}// end if
				}
		}	else 
		{
			if (endCh != '$')
			{
				if (ch == endCh)
					continue;
			}
			 Out += ch;
		}

	} //  end while

	return Out;
} 