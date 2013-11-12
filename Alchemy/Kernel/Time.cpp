//	Time.cpp
//
//	Time package

#include "Kernel.h"

#define FORMAT_INTERNET							CONSTLIT("internet")

int g_DaysInMonth[] =
	//	J   F   M   A   M   J   J   A   S   O   N   D
	{	31,	28,	31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int g_DaysOfYearByMonth[] =
	//		J   F   M   A   M   J   J   A   S   O   N   D
	{	0,  31, 59, 90, 120,151,181,212,243,273,304,334,365 };

int g_DaysOfLeapYearByMonth[] =
	//		J   F   M   A   M   J   J   A   S   O   N   D
	{	0,  31, 60, 91, 121,152,182,213,244,274,305,335,366 };

char *g_szMonthName[] =
	{
	"",	"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
	};

char *g_szMonthNameShort[] =
	{
	"",	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

char *g_szDayNameShort[] =
	{
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};

//	CTimeDate -----------------------------------------------------------------

CTimeDate::CTimeDate (void)

//	CTimeDate constructor

	{
	m_Time.wDay = 1;
	m_Time.wMonth = 1;
	m_Time.wYear = 1;
	m_Time.wHour = 0;
	m_Time.wMinute = 0;
	m_Time.wSecond = 0;
	m_Time.wMilliseconds = 0;
	m_Time.wDayOfWeek = 0xffff;
	}

CTimeDate::CTimeDate (Constants Init)

//	CTimeDate constructor

	{
	switch (Init)
		{
		case Now:
			::GetSystemTime(&m_Time);
			break;

		//	WARNING: Not sure if this works for timezones other than GMT

		case Today:
			::GetSystemTime(&m_Time);

			m_Time.wHour = 0;
			m_Time.wMinute = 0;
			m_Time.wSecond = 0;
			m_Time.wMilliseconds = 0;
			break;

		default:
			ASSERT(false);
		};
	}

CTimeDate::CTimeDate (const SYSTEMTIME &Time)

//	CTimeDate constructor

	{
	m_Time = Time;
	}

CTimeDate::CTimeDate (int iDaysSince1AD, int iMillisecondsSinceMidnight)

//	CTimeDate constructor

	{
	//	Take a stab at figuring out the year

	int iY1 = (400 * iDaysSince1AD - 146000) / 146097;
	
	int iDays1 = 0;
	if (iY1 > 0)
		{
		iDays1 += 365 * iY1;
		iDays1 += iY1 / 4;
		iDays1 -= iY1 / 100;
		iDays1 += iY1 / 400;
		}

	iDays1 = iDaysSince1AD - iDays1;

	//	Adjust if we're over

	int iDaysInY1 = (timeIsLeapYear(iY1+1) ? 366 : 365);
	while (iDays1 > iDaysInY1)
		{
		iY1++;
		iDays1 -= iDaysInY1;
	
		iDaysInY1 = (timeIsLeapYear(iY1+1) ? 366 : 365);
		}

	m_Time.wYear = (iY1 + 1);

	//	Calculate month and day

	m_Time.wMonth = 1;
	if (timeIsLeapYear(m_Time.wYear))
		{
		while (iDays1 + 1 > g_DaysOfLeapYearByMonth[m_Time.wMonth])
			m_Time.wMonth++;

		m_Time.wDay = (iDays1 - g_DaysOfLeapYearByMonth[m_Time.wMonth-1]) + 1;
		}
	else
		{
		while (iDays1 + 1 > g_DaysOfYearByMonth[m_Time.wMonth])
			m_Time.wMonth++;

		m_Time.wDay = (iDays1 - g_DaysOfYearByMonth[m_Time.wMonth-1]) + 1;
		}

	//	Calculate time

	m_Time.wHour = iMillisecondsSinceMidnight / (60 * 60 * 1000);
	m_Time.wMinute = (iMillisecondsSinceMidnight % (60 * 60 * 1000)) / (60 * 1000);
	m_Time.wSecond = (iMillisecondsSinceMidnight % (60 * 1000)) / 1000;
	m_Time.wMilliseconds = iMillisecondsSinceMidnight % 1000;

	//	Don't yet calculate day of week

	m_Time.wDayOfWeek = 0xffff;
	}

int CTimeDate::Compare (const CTimeDate &Src) const

//	Compare
//
//	If this > Src,		1
//	If this == Src,		0
//	If this < Src,		-1

	{
	if (m_Time.wYear > Src.m_Time.wYear)
		return 1;
	else if (m_Time.wYear < Src.m_Time.wYear)
		return -1;
	else if (m_Time.wMonth > Src.m_Time.wMonth)
		return 1;
	else if (m_Time.wMonth < Src.m_Time.wMonth)
		return -1;
	else if (m_Time.wDay > Src.m_Time.wDay)
		return 1;
	else if (m_Time.wDay < Src.m_Time.wDay)
		return -1;
	else if (m_Time.wHour > Src.m_Time.wHour)
		return 1;
	else if (m_Time.wHour < Src.m_Time.wHour)
		return -1;
	else if (m_Time.wMinute > Src.m_Time.wMinute)
		return 1;
	else if (m_Time.wMinute < Src.m_Time.wMinute)
		return -1;
	else if (m_Time.wSecond > Src.m_Time.wSecond)
		return 1;
	else if (m_Time.wSecond < Src.m_Time.wSecond)
		return -1;
	else if (m_Time.wMilliseconds > Src.m_Time.wMilliseconds)
		return 1;
	else if (m_Time.wMilliseconds < Src.m_Time.wMilliseconds)
		return -1;
	else
		return 0;
	}

int CTimeDate::DayOfWeek (void) const

//	DayOfWeek
//
//	Returns the day of week of the current date. 0 = Sunday.
//	See: http://www.faqs.org/faqs/calendars/faq/part1/index.html

	{
	int a = (14 - m_Time.wMonth) / 12;
	int y = m_Time.wYear - a;
	int m = m_Time.wMonth + 12 * a - 2;

	//	LATER: We assume Gregorian calendar

	return (m_Time.wDay + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12) % 7;
	}

int CTimeDate::DaysSince1AD (void) const

//	DaysSince1AD
//
//	Returns the number of days elapsed since Jan 1, 1 AD.

	{
	int iDays = 0;

	//	Add all years before this one

	iDays += 365 * (Year() - 1);
	iDays += (Year() - 1) / 4;
	iDays -= (Year() - 1) / 100;
	iDays += (Year() - 1) / 400;

	//	Now add the day months for this year

	if (timeIsLeapYear(Year()))
		iDays += g_DaysOfLeapYearByMonth[Month()-1];
	else
		iDays += g_DaysOfYearByMonth[Month()-1];

	iDays += Day()-1;

	return iDays;
	}

void WriteNumber (CMemoryWriteStream &Stream, int iNumber, int iLeadingZeros)
	{
	CString sNumber = strFromInt(iNumber, false);
	if (sNumber.GetLength() < iLeadingZeros)
		Stream.Write("0000000000", iLeadingZeros - sNumber.GetLength());

	Stream.Write(sNumber.GetASCIIZPointer(), sNumber.GetLength());
	}

CString CTimeDate::Format (const CString &sFormat) const

//	Format
//
//	Formats a timedate
//
//    %a    abbreviated weekday name (Sun)
//    %A    full weekday name (Sunday)
//    %b    abbreviated month name (Dec)
//    %B    full month name (December)
//    %c    date and time (Dec  2 06:55:15 1979)
//    %d    day of the month (02)
//    %H    hour of the 24-hour day (06)
//    %I    hour of the 12-hour day (06)
//    %j    day of the year, from 001 (335)
//    %m    month of the year, from 01 (12)
//    %M    minutes after the hour (55)
//    %p    AM/PM indicator (AM)
//    %S    seconds after the minute (15)
//    %U    Sunday week of the year, from 00 (48)
//    %w    day of the week, from 0 for Sunday (6)
//    %W    Monday week of the year, from 00 (47)
//    %x    date (Dec  2 1979)
//    %X    time (06:55:15)
//    %y    year of the century, from 00 (79)
//    %Y    year (1979)
//    %Z    time zone name, if any (EST)
//    %%    percent character %

	{
	//	Internet format:
	//
	//	Sun, 06 Nov 1994 08:49:37 GMT

	if (strEquals(sFormat, FORMAT_INTERNET))
		{
		return strPatternSubst(CONSTLIT("%s, %02d %s %d %02d:%02d:%02d GMT"),
				CString(g_szDayNameShort[DayOfWeek()], 3, true),
				m_Time.wDay,
				CString(g_szMonthNameShort[m_Time.wMonth], 3, true),
				m_Time.wYear,
				m_Time.wHour,
				m_Time.wMinute,
				m_Time.wSecond);
		}

	//	Otherwise we expect various fields

	else
		{
		CMemoryWriteStream Stream(0);
		if (Stream.Create() != NOERROR)
			return NULL_STR;

		char *pPos = sFormat.GetASCIIZPointer();
		while (*pPos != '\0')
			{
			if (*pPos == '%')
				{
				pPos++;

				int iLeadingZeros;
				if (*pPos == '0')
					{
					pPos++;

					if (*pPos >= '1' && *pPos <= '9')
						{
						iLeadingZeros = (*pPos) - '0';
						pPos++;
						}
					else
						iLeadingZeros = 2;
					}
				else
					iLeadingZeros = 0;
			
				switch (*pPos)
					{
					case '\0':
						break;

					case 'B':
						Stream.Write(g_szMonthName[m_Time.wMonth], (int)::strlen(g_szMonthName[m_Time.wMonth]));
						pPos++;
						break;

					case 'd':
						WriteNumber(Stream, m_Time.wDay, iLeadingZeros);
						pPos++;
						break;

					case 'I':
						if ((m_Time.wHour % 12) == 0)
							WriteNumber(Stream, 12, iLeadingZeros);
						else
							WriteNumber(Stream, m_Time.wHour % 12, iLeadingZeros);
						pPos++;
						break;

					case 'M':
						WriteNumber(Stream, m_Time.wMinute, 2);
						pPos++;
						break;

					case 'p':
						if (m_Time.wHour < 12)
							Stream.Write("AM", 2);
						else
							Stream.Write("PM", 2);
						pPos++;
						break;

					case 'S':
						WriteNumber(Stream, m_Time.wSecond, 2);
						pPos++;
						break;

					case 'Y':
						WriteNumber(Stream, m_Time.wYear, 0);
						pPos++;
						break;

					default:
						{
						Stream.Write(pPos, 1);
						pPos++;
						}
					}
				}
			else
				{
				Stream.Write(pPos, 1);
				pPos++;
				}
			}

		return CString(Stream.GetPointer(), Stream.GetLength());
		}
	}

int CTimeDate::MillisecondsSinceMidnight (void) const

//	MillisecondsSinceMidnight
//
//	Returns the number of milliseconds since midnight

	{
	return Millisecond()
			+ (Second() * 1000)
			+ (Minute() * 60 * 1000)
			+ (Hour() * 60 * 60 * 1000);
	}

bool CTimeDate::Parse (const CString &sFormat, const CString &sValue, CString *retsError)

//	Parse
//
//	Parse a date of the given format.

	{
	//	Internet format:
	//
	//	Sun, 06 Nov 1994 08:49:37 GMT

	if (strEquals(sFormat, FORMAT_INTERNET))
		{
		char *pPos = sValue.GetASCIIZPointer();
		char *pPosEnd = pPos + sValue.GetLength();

		//	Skip leading whitespace

		while (strIsWhitespace(pPos))
			pPos++;

		//	Skip day of week

		while (pPos < pPosEnd
				&& (*pPos < '0' || *pPos > '9'))
			pPos++;

		if (pPos >= pPosEnd)
			goto InvalidValue;

		//	Day

		bool bFail;
		m_Time.wDay = strParseInt(pPos, 0, &pPos, &bFail);
		if (bFail || m_Time.wDay < 1 || m_Time.wDay > 31)
			goto InvalidValue;

		pPos++;
		if (pPos >= pPosEnd)
			goto InvalidValue;

		//	Month

		if (*pPos == 'A')
			{
			pPos++;
			if (*pPos == 'p')
				{
				m_Time.wMonth = 4;
				pPos += 3;
				}
			else if (*pPos == 'u')
				{
				m_Time.wMonth = 8;
				pPos += 3;
				}
			else
				goto InvalidValue;
			}
		else if (*pPos == 'D')
			{
			m_Time.wMonth = 12;
			pPos += 4;
			}
		else if (*pPos == 'F')
			{
			m_Time.wMonth = 2;
			pPos += 4;
			}
		else if (*pPos == 'J')
			{
			pPos++;
			if (*pPos == 'a')
				{
				m_Time.wMonth = 1;
				pPos += 3;
				}
			else if (*pPos == 'u')
				{
				pPos++;
				if (*pPos == 'l')
					{
					m_Time.wMonth = 7;
					pPos += 2;
					}
				else if (*pPos == 'n')
					{
					m_Time.wMonth = 6;
					pPos += 2;
					}
				else
					goto InvalidValue;
				}
			else
				goto InvalidValue;
			}
		else if (*pPos == 'M')
			{
			pPos++;
			if (*pPos == 'a')
				{
				pPos++;
				if (*pPos == 'r')
					{
					m_Time.wMonth = 3;
					pPos += 2;
					}
				else if (*pPos == 'y')
					{
					m_Time.wMonth = 5;
					pPos += 2;
					}
				else
					goto InvalidValue;
				}
			else
				goto InvalidValue;
			}
		else if (*pPos == 'N')
			{
			m_Time.wMonth = 11;
			pPos += 4;
			}
		else if (*pPos == 'O')
			{
			m_Time.wMonth = 10;
			pPos += 4;
			}
		else if (*pPos == 'S')
			{
			m_Time.wMonth = 9;
			pPos += 4;
			}
		else
			goto InvalidValue;

		if (pPos >= pPosEnd)
			goto InvalidValue;

		//	Year

		m_Time.wYear = strParseInt(pPos, 0, &pPos, &bFail);
		if (bFail || m_Time.wYear < 1)
			goto InvalidValue;

		pPos++;
		if (pPos >= pPosEnd)
			goto InvalidValue;

		//	Hour

		m_Time.wHour = strParseInt(pPos, 0, &pPos, &bFail);
		if (bFail || m_Time.wHour > 23)
			goto InvalidValue;

		pPos++;
		if (pPos >= pPosEnd)
			goto InvalidValue;

		//	Minute

		m_Time.wMinute = strParseInt(pPos, 0, &pPos, &bFail);
		if (bFail || m_Time.wMinute > 59)
			goto InvalidValue;

		pPos++;
		if (pPos >= pPosEnd)
			goto InvalidValue;

		//	Second

		m_Time.wSecond = strParseInt(pPos, 0, &pPos, &bFail);
		if (bFail || m_Time.wSecond > 59)
			goto InvalidValue;

		//	Millisecond is not stored

		m_Time.wMilliseconds = 0;

		//	We don't store day of week

		m_Time.wDayOfWeek = 0xffff;

		//	Done

		return true;

InvalidValue:

		if (retsError) *retsError = strPatternSubst(CONSTLIT("Invalid date value: %s"), sValue);
		return false;
		}
	else
		{
		if (retsError) *retsError = strPatternSubst(CONSTLIT("Unsupported date format: %s."), sFormat);
		return false;
		}
	}

CTimeDate CTimeDate::ToLocalTime (void) const

//	ToLocalTime
//
//	Returns the current time as local time

	{
	SYSTEMTIME LocalTime;
	::SystemTimeToTzSpecificLocalTime(NULL, (SYSTEMTIME *)&m_Time, &LocalTime);
	return CTimeDate(LocalTime);
	}

//	CTimeSpan -----------------------------------------------------------------

CTimeSpan::CTimeSpan (void) : m_Days(0), m_Milliseconds(0)

//	CTimeSpan constructor

	{
	}

CTimeSpan::CTimeSpan (int iMilliseconds)

//	CTimeSpan constructor

	{
	m_Days = iMilliseconds / (SECONDS_PER_DAY * 1000);
	m_Milliseconds = iMilliseconds % (SECONDS_PER_DAY * 1000);
	}

CTimeSpan::CTimeSpan (int iDays, int iMilliseconds) : m_Days(iDays), m_Milliseconds(iMilliseconds)

//	CTimeSpan constructor

	{
	}

CString CTimeSpan::Encode (void) const

//	Encode
//
//	Save as two integers. This can be revered with Parse().

	{
	return strPatternSubst(CONSTLIT("D:0x%x M:0x%x"), m_Days, m_Milliseconds);
	}

CString FormatTwoUnits (const CString &sMajor, const CString &sMinor, int iMinor)
	{
	if (iMinor == 0)
		return sMajor;
	else
		return strPatternSubst(CONSTLIT("%s and %s"), sMajor, sMinor);
	}

CString CTimeSpan::Format (const CString &sFormat) const

//	Format
//
//	Formats the time span
//
//	-			1 hour and 5 minutes
//	hh:mm:ss	04:01:10.4

	{
	int iYears = m_Days / 365;
	int iDays = m_Days % 365;
	int iHours = m_Milliseconds / (60 * 60 * 1000);
	int iMinutes = (m_Milliseconds % (60 * 60 * 1000)) / (60 * 1000);
	int iSeconds = (m_Milliseconds % (60 * 1000)) / 1000;
	int iMilliseconds = (m_Milliseconds % 1000);

	CString sYears;
	if (iYears == 1)
		sYears = CONSTLIT("1 year");
	else if (iYears > 1)
		sYears = strPatternSubst(CONSTLIT("%d years"), iYears);

	CString sDays;
	if (iDays == 1)
		sDays = CONSTLIT("1 day");
	else if (iDays > 1)
		sDays = strPatternSubst(CONSTLIT("%d days"), iDays);

	CString sHours;
	if (iHours == 1)
		sHours = CONSTLIT("1 hour");
	else if (iHours > 1)
		sHours = strPatternSubst(CONSTLIT("%d hours"), iHours);

	CString sMinutes;
	if (iMinutes == 1)
		sMinutes = CONSTLIT("1 minute");
	else if (iMinutes > 1)
		sMinutes = strPatternSubst(CONSTLIT("%d minutes"), iMinutes);

	CString sSeconds;
	if (iMinutes == 0 && iHours == 0 && iDays == 0 && iYears == 0)
		sSeconds = strPatternSubst(CONSTLIT("%d.%d seconds"), iSeconds, (iMilliseconds + 50) / 100);
	else if (iSeconds == 1)
		sSeconds = CONSTLIT("1 second");
	else
		sSeconds = strPatternSubst(CONSTLIT("%d seconds"), iSeconds + (iMilliseconds >= 500 ? 1 : 0));

	//	Format

	CString sResult;
	if (iYears > 0)
		return FormatTwoUnits(sYears, sDays, iDays);
	else if (iDays > 0)
		return FormatTwoUnits(sDays, sHours, iHours);
	else if (iHours > 0)
		return FormatTwoUnits(sHours, sMinutes, iMinutes);
	else if (iMinutes > 0)
		return FormatTwoUnits(sMinutes, sSeconds, iSeconds);
	else
		return sSeconds;
	}

bool CTimeSpan::Parse (const CString &sValue, CTimeSpan *retValue)

//	Parse
//
//	Parses a string of the form:
//
//	D:0xddd M:0xmmm
//		(where ddd = days; mmm = milliseconds)

	{
	char *pPos = sValue.GetASCIIZPointer();

	//	If the first character is a 'D' then we assume an encoded TimeSpan

	if (*pPos == 'D')
		{
		pPos++;
		if (*pPos++ != ':')
			return false;

		bool bFailed;
		DWORD dwDays = strParseInt(pPos, 0, &pPos, &bFailed);
		if (bFailed)
			return false;

		if (*pPos++ != ' ')
			return false;

		if (*pPos++ != 'M')
			return false;

		if (*pPos++ != ':')
			return false;

		DWORD dwMilliseconds = strParseInt(pPos, 0, &pPos, &bFailed);
		if (bFailed)
			return false;

		*retValue = CTimeSpan(dwDays, dwMilliseconds);
		return true;
		}

	//	Otherwise we try to parse a plain English time span

	else
		{
		DWORD dwTotalDays = 0;
		DWORD dwTotalMilliseconds = 0;

		//	Parse components

		DWORD dwDays;
		DWORD dwMilliseconds;
		if (!ParsePartial(pPos, &dwDays, &dwMilliseconds, &pPos))
			return false;

		dwTotalDays += dwDays;
		dwTotalMilliseconds += dwMilliseconds;

		//	Look for 'and'

		while (*pPos == ' ')
			pPos++;

		if (*pPos++ == 'a')
			{
			if (*pPos++ != 'n')
				return false;
			if (*pPos++ != 'd')
				return false;

			if (!ParsePartial(pPos, &dwDays, &dwMilliseconds, &pPos))
				return false;

			dwTotalDays += dwDays;
			dwTotalMilliseconds += dwMilliseconds;
			}

		//	Done

		*retValue = CTimeSpan(dwTotalDays, dwTotalMilliseconds);
		return true;
		}
	}

bool CTimeSpan::ParsePartial (char *pPos, DWORD *retdwDays, DWORD *retdwMilliseconds, char **retpPos)

//	ParsePartial
//
//	Parses a string of the form:
//
//	xx {time-unit}

	{
	//	Skip whitespace

	while (*pPos == ' ')
		pPos++;

	//	Parse a number

	bool bFailed;
	DWORD dwValue1 = strParseInt(pPos, 0, &pPos, &bFailed);
	if (bFailed)
		return false;

	//	See if we have a decimal

	DWORD dwValue2;
	if (*pPos == '.')
		{
		pPos++;
		dwValue2 = strParseInt(pPos, 0, &pPos, &bFailed);
		if (bFailed)
			return false;

		//	Decimal extend to three digits. E.g., if .7, we convert to 700.

		if (dwValue2 < 10)
			dwValue2 *= 100;
		else if (dwValue2 < 100)
			dwValue2 *= 10;
		}
	else
		dwValue2 = 0;

	//	Skip whitespace

	while (*pPos == ' ')
		pPos++;

	//	Get the time unit

	char *pStart = pPos;
	while (*pPos != ' ' && *pPos != '\0')
		pPos++;

	//	We can guess the time unit based on the first character

	if (*pStart == 'y')
		{
		*retdwDays = dwValue1 * 365;
		*retdwMilliseconds = 0;
		}
	else if (*pStart == 'd')
		{
		*retdwDays = dwValue1;
		*retdwMilliseconds = 0;
		}
	else if (*pStart == 'h')
		{
		*retdwDays = 0;
		*retdwMilliseconds = dwValue1 * 60 * 60 * 1000;
		}
	else if (*pStart == 'm')
		{
		*retdwDays = 0;
		*retdwMilliseconds = dwValue1 * 60 * 1000;
		}
	else if (*pStart == 's')
		{
		*retdwDays = 0;
		*retdwMilliseconds = dwValue1 * 1000 + dwValue2;
		}
	else
		return false;

	//	Done

	*retpPos = pPos;
	return true;
	}

void CTimeSpan::ReadFromStream (IReadStream *pStream)

//	ReadFromStream
//
//	Reads from a stream

	{
	pStream->Read((char *)&m_Days, sizeof(DWORD));
	pStream->Read((char *)&m_Milliseconds, sizeof(DWORD));
	}

void CTimeSpan::WriteToStream (IWriteStream *pStream) const

//	WriteToStream
//
//	Writes out to a stream

	{
	pStream->Write((char *)&m_Days, sizeof(DWORD));
	pStream->Write((char *)&m_Milliseconds, sizeof(DWORD));
	}

//	Functions -----------------------------------------------------------------

const CTimeSpan operator+ (const CTimeSpan &op1, const CTimeSpan &op2)

//	Operator -
//
//	Subtract op2 from op1 (op1 must be greater than op2)

	{
	LONGLONG time1 = (op1.Days() * SECONDS_PER_DAY * 1000) + op1.Milliseconds();
	LONGLONG time2 = (op2.Days() * SECONDS_PER_DAY * 1000) + op2.Milliseconds();
	LONGLONG result = time1 + time2;

	return CTimeSpan((int)(result / (SECONDS_PER_DAY * 1000)), (int)(result % (SECONDS_PER_DAY * 1000)));
	}

const CTimeSpan operator- (const CTimeSpan &op1, const CTimeSpan &op2)

//	Operator -
//
//	Subtract op2 from op1 (op1 must be greater than op2)

	{
	LONGLONG time1 = (op1.Days() * SECONDS_PER_DAY * 1000) + op1.Milliseconds();
	LONGLONG time2 = (op2.Days() * SECONDS_PER_DAY * 1000) + op2.Milliseconds();
	LONGLONG result = (time1 >= time2 ? time1 - time2 : time2 - time1);

	return CTimeSpan((int)(result / (SECONDS_PER_DAY * 1000)), (int)(result % (SECONDS_PER_DAY * 1000)));
	}

CTimeDate timeAddTime (const CTimeDate &StartTime, const CTimeSpan &Addition)

//	timeAddTime
//
//	Adds a timespan to a timedate

	{
	int iDaysSince1AD = StartTime.DaysSince1AD();
	int iMillisecondsSinceMidnight = StartTime.MillisecondsSinceMidnight();

	//	Add

	iDaysSince1AD += Addition.Days();
	iMillisecondsSinceMidnight += Addition.MillisecondsSinceMidnight();
	if (iMillisecondsSinceMidnight >= SECONDS_PER_DAY * 1000)
		{
		iDaysSince1AD++;
		iMillisecondsSinceMidnight -= SECONDS_PER_DAY * 1000;
		}

	return CTimeDate(iDaysSince1AD, iMillisecondsSinceMidnight);
	}

bool timeIsLeapYear (int iYear)

//	timeIsLeapYear
//
//	Returns TRUE if given year is a leap year

	{
	return ((iYear % 4) == 0)
			&& (((iYear % 100) != 0) || ((iYear % 400) == 0));
	}

CTimeSpan timeSpan (const CTimeDate &StartTime, const CTimeDate &EndTime)

//	timeSpan
//
//	Returns the difference between the two times

	{
	int iDays = EndTime.DaysSince1AD() - StartTime.DaysSince1AD();
	if (iDays < 0)
		return timeSpan(EndTime, StartTime);

	int iStartTime = StartTime.MillisecondsSinceMidnight();
	int iEndTime = EndTime.MillisecondsSinceMidnight();

	int iMilliseconds = 0;
	if (iEndTime > iStartTime)
		{
		iMilliseconds = iEndTime - iStartTime;
		}
	else
		{
		if (iDays > 0)
			{
			iDays--;
			iMilliseconds = (iEndTime + SECONDS_PER_DAY * 1000) - iStartTime;
			}
		else
			iMilliseconds = iStartTime - iEndTime;
		}

	return CTimeSpan(iDays, iMilliseconds);
	}

CTimeDate timeSubtractTime (const CTimeDate &StartTime, const CTimeSpan &Subtraction)

//	timeSubtractTime
//
//	Subtracts time from timedate

	{
	int iDaysSince1AD = StartTime.DaysSince1AD();
	int iMillisecondsSinceMidnight = StartTime.MillisecondsSinceMidnight();

	//	Add

	iDaysSince1AD -= Subtraction.Days();
	if (Subtraction.MillisecondsSinceMidnight() > iMillisecondsSinceMidnight)
		{
		iMillisecondsSinceMidnight += SECONDS_PER_DAY * 1000;
		iDaysSince1AD--;
		}
	
	iMillisecondsSinceMidnight -= Subtraction.MillisecondsSinceMidnight();

	return CTimeDate(iDaysSince1AD, iMillisecondsSinceMidnight);
	}

