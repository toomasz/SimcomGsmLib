#pragma once

#include <SSD1306.h>
#include <SimcomAtCommands.h>
#include <FixedString.h>
#include <GsmModule.h>
#include <GsmLibHelpers.h>

enum class Font
{
	F10,
	F16
};

class Gui
{
	SSD1306 _lcd;
public:
	Gui(uint8_t i2cAddress, uint8_t sda, uint8_t scl):
	_lcd(i2cAddress, sda, scl)
	{

	}
	void init()
	{
		if (!_lcd.init())
		{
			Serial.println("Failed to init lcd");
		}
		else
		{
			Serial.println("Lcd initialized successfully");
		}
		Wire.setClock(100000);
		_lcd.resetDisplay();
		_lcd.setFont(ArialMT_Plain_16);
		_lcd.clear();
	}
	void drawBatterySymbol(int percent, float voltage)
	{
		const int width = 12;
		const int height = 25;
		const int batteryPlusHeight = 3;
		const int batteryPlusPadding = 2;

		_lcd.setColor(OLEDDISPLAY_COLOR::BLACK);

		_lcd.fillRect(0, 0, width, height);
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);

		// main battery rect
		_lcd.drawRect(0, batteryPlusHeight, width, height - batteryPlusHeight);

		// battery plus rect
		_lcd.drawRect(batteryPlusPadding, 0,
			width - 2 * batteryPlusPadding, batteryPlusHeight + 1);

		_lcd.setColor(OLEDDISPLAY_COLOR::BLACK);
		_lcd.drawHorizontalLine(batteryPlusPadding + 1, batteryPlusHeight,
			width - 2 * (batteryPlusPadding + 1));

		int fillHeightWhen100Percent = height - batteryPlusHeight - 2;

		int batteryUsedInPercent = 100 - percent;

		int batteryLeftPixels = ((double)batteryUsedInPercent * (double)fillHeightWhen100Percent) / 100.0;



		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.fillRect(2, batteryPlusHeight + 2 + batteryLeftPixels, width - 2 * 2, fillHeightWhen100Percent - 2 - batteryLeftPixels);

	}

	void DisplaySocketState(GsmAsyncSocket *socket)
	{
		lcd_label(Font::F10, 0, 64 - 22, F("%s"), SocketStateToStr(socket->GetState()));
		lcd_label(Font::F10, 0, 64 - 12, F("r/s: %llu b/%llu b"), socket->GetReceivedBytes(), socket->GetSentBytes());
	}

	void DisplayGsmState(GsmModule& gsm)
	{
		auto state = gsm.GetState();
		if (state == GsmState::NoShield)
		{
			FixedString20 error = "No shield";
			DisplayError(error);
			return;
		}

		if (state == GsmState::SimError)
		{
			DisplaySimError(gsm.simStatus);
			return;
		}

		drawBattery(gsm.batteryInfo.Percent, gsm.batteryInfo.Voltage);
		drawGsmInfo(gsm.signalQuality, gsm.gsmRegStatus, gsm.operatorName);
		DisplayBlinkIndicator();

		if (state == GsmState::ConnectingToGprs)
		{
			lcd_label(Font::F10, 0, 32, F("Connecting to gprs.."));
		}
		if (state == GsmState::Initializing)
		{
			lcd_label(Font::F10, 0, 32, F("Initializing modem..."));
		}
		if (state == GsmState::ConnectedToGprs)
		{
			DisplayIp(gsm.ipAddress);
		}

		if(state == GsmState::Error)
		{
			DrawFramePopup(gsm.Error(), 40, 5);
		}

		DisplayIncomingCall(gsm.callInfo);
	}
	void drawBattery(int percent, float voltage)
	{
		drawBatterySymbol(percent, voltage);

		const int paddingLeft = 14;
		const int leftTextPadding = 2;
		int textX = paddingLeft + leftTextPadding;

		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.setFont(ArialMT_Plain_16);

		char buffer[20];
		snprintf_P(buffer, 20, (PGM_P)F("%d%%"), percent);
		_lcd.drawString(textX, 0, buffer);
		_lcd.setFont(ArialMT_Plain_10);

		snprintf_P(buffer, 20, (PGM_P)F("%.2f V"), voltage);
		_lcd.drawString(textX, 14, buffer);
	}


	void drawSignalQuality(int csqQuality)
	{
		const double triangleWidth = 28;
		const double triangleHeight = 16;

		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.drawHorizontalLine(128 - triangleWidth, triangleHeight, triangleWidth);
		_lcd.drawVerticalLine(128 - 1, 0, triangleHeight);
		_lcd.drawLine(128 - triangleWidth, triangleHeight, 128 - 1, 0);

		auto qualityToPixel = (double)csqQuality / 32.0 * triangleWidth;

		for (int i = 0; i < qualityToPixel; i++)
		{
			int lineHeight = (double)i / triangleWidth * triangleHeight;
			_lcd.drawVerticalLine(128-triangleWidth + i, triangleHeight - lineHeight, lineHeight);
		}
		_lcd.setFont(ArialMT_Plain_10);

		char buffer[10];
		snprintf_P(buffer, 10, (PGM_P)F("%d"), csqQuality);
		_lcd.drawString(128 - triangleWidth-2, 0, buffer);
	}

	const char* RegistrationStatusToStr(GsmRegistrationState regStatus)
	{
		switch (regStatus)
		{
		case GsmRegistrationState::SearchingForNetwork:
			return "Net Search...";
		case GsmRegistrationState::RegistrationDenied:
			return "Reg denied";
		case GsmRegistrationState::RegistrationUnknown:
			return "Reg unknown";
		default:
			break;
		}
		return "";
	}

	void lcd_label(Font fontSize, int x, int y, const __FlashStringHelper* format, ...)
	{
		va_list argptr;
		va_start(argptr, format);

		if (fontSize == Font::F16)
		{
			_lcd.setFont(ArialMT_Plain_16);
		}
		if (fontSize == Font::F10)
		{
			_lcd.setFont(ArialMT_Plain_10);
		}

		char buffer[200];
		vsnprintf_P(buffer, 200, (PGM_P)format, argptr);
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.drawString(x, y, buffer);
		va_end(argptr);
	}

	void drawGsmInfo(int signalQuality, GsmRegistrationState gsmNetworkStatus, FixedString20& operatorName)
	{
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		if (gsmNetworkStatus == GsmRegistrationState::HomeNetwork || 
			gsmNetworkStatus == GsmRegistrationState::Roaming)
		{
			_lcd.setFont(ArialMT_Plain_10);
			auto opWidth = _lcd.getStringWidth(operatorName.c_str());
			_lcd.drawString(128 - opWidth, 16, operatorName.c_str());
			drawSignalQuality(signalQuality);
			return;
		}
		
		auto gsmStatusStr = RegistrationStatusToStr(gsmNetworkStatus);
		auto statusStrWidth = _lcd.getStringWidth(gsmStatusStr);
		_lcd.drawString(128 - statusStrWidth, 17, gsmStatusStr);
	}

	void DisplayIncomingCall(IncomingCallInfo &callInfo)
	{
		if (callInfo.HasIncomingCall)
		{
			_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
			_lcd.drawRect(5, 10, 128 - 10, 45);
			_lcd.setColor(OLEDDISPLAY_COLOR::BLACK);
			_lcd.fillRect(6, 11, 128 - 10 - 2, 45 - 2);

			lcd_label(Font::F16, 10, 14, F("Calling: "));
			lcd_label(Font::F10, 10, 32, F("%s"), callInfo.CallerNumber.c_str());
		}
	}

	void DisplayIp(GsmIp& ip)
	{
		lcd_label(Font::F10, 0,26, F("ip: %s"), ip.ToString().c_str());
	}	

	void Clear()
	{
		_lcd.setColor(OLEDDISPLAY_COLOR::BLACK);
		_lcd.fillRect(0, 0, 128, 64);
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
	}

	void Display()
	{
		_lcd.display();
	}

	void DisplayBlinkIndicator()
	{
		static bool rectState = false;

		static IntervalTimer blinkIntervalTimer(200);
		if(blinkIntervalTimer.IsElapsed())
		{
			rectState = !rectState;
		}
		auto color = rectState ? OLEDDISPLAY_COLOR::WHITE : OLEDDISPLAY_COLOR::BLACK;
		_lcd.setColor(color);
		const int rectSize= 10;
		_lcd.fillCircle(128 - 50, 8, 8);
	}

	void DisplaySimError(SimState simStatus)
	{
		FixedString50 error;
		if (simStatus == SimState::Locked)
		{
			error = F("Sim car is locked by PIN");
		}
		else if (simStatus == SimState::NotInserted)
		{
			error = F("Sim card not inserted");
		}
		else
		{
			error = F("Other sim card problem");
		}
		DisplayError(error);
	}

	void DisplayError(FixedString100 error)
	{
		Clear();
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.drawRect(5, 10, 128 - 10, 45);
		_lcd.setColor(OLEDDISPLAY_COLOR::BLACK);
		_lcd.fillRect(6, 11, 128 - 10 - 2, 45 - 2);
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.setFont(ArialMT_Plain_10);
		_lcd.drawStringMaxWidth(10, 14, 128 - 10 - 2, error.c_str());
		_lcd.display();
	}

	void DrawFramePopup(FixedStringBase &message, int paddingTop = 15, int paddingBottom = 15, int paddingSide = 10)
	{
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.drawRect(paddingSide, paddingTop, 128 - paddingSide*2, 64 - paddingTop-paddingBottom);
		_lcd.setColor(OLEDDISPLAY_COLOR::BLACK);
		_lcd.fillRect(paddingSide +1, paddingTop +1 , (128 - paddingSide * 2) - 2, 64- paddingTop - paddingBottom - 2);
		_lcd.setColor(OLEDDISPLAY_COLOR::WHITE);
		_lcd.setFont(ArialMT_Plain_10);
		_lcd.drawString(paddingSide + 4, paddingTop + 4 , message.c_str());
	}
};

