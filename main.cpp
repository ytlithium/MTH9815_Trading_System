// main.cpp
//
// Purpose: 1. Test all the functionalities of the trading system.
// 
// @author Yuanting Li
// @version 1.0 2023/12/24

#include <iostream>
#include <string>
#include <iomanip>
#include <filesystem>

#include "soa.hpp"
#include "products.hpp"
#include "marketdataservice.hpp"
#include "pricingservice.hpp"
#include "riskservice.hpp"
#include "executionservice.hpp"
#include "positionservice.hpp"
#include "inquiryservice.hpp"
#include "historicaldataservice.hpp"
#include "streamingservice.hpp"
#include "algostreamingservice.hpp"
#include "tradebookingservice.hpp"
#include "algoexecutionservice.hpp"
#include "guiservice.hpp"
#include "utilities.hpp"

using namespace std;

int main(){

	// ----- Data Path Setup -----
	string dataPath = "./data";
	// Create or clean data folder
	if (filesystem::exists(dataPath)) {
		filesystem::remove_all(dataPath);
	}
	filesystem::create_directory(dataPath);

	string resPath = "./result";
	// Create or clean results folder
	if (filesystem::exists(resPath)) {
		filesystem::remove_all(resPath);
	}
	filesystem::create_directory(resPath);

	// Define paths for different data files
	const string pricePath = "./data/prices.txt";
	const string marketDataPath = "./data/marketdata.txt";
	const string tradePath = "./data/trades.txt";
	const string inquiryPath = "./data/inquiries.txt";

	// ----- Data Generation -----
	log(LogLevel::INFO, "Generating price and orderbook data...");
	vector<string> bonds = { "9128283H1", "9128283L2", "912828M80", "9128283J7", "9128283F5", "912810TW8", "912810RZ3" };
	genOrderBook(bonds, pricePath, marketDataPath, 39373, 100,0000);
	genTrades(bonds, tradePath, 39373);
	genInquiries(bonds, inquiryPath, 39373);
	log(LogLevel::INFO, "Data generation complete.");

    // ----- create services -----
    log(LogLevel::INFO, "Initializing service components...");
	PricingService<Bond> pricingService;
	AlgoStreamingService<Bond> algoStreamingService;
	StreamingService<Bond> streamingService;
	MarketDataService<Bond> marketDataService;
	AlgoExecutionService<Bond> algoExecutionService;
	ExecutionService<Bond> executionService;
	TradeBookingService<Bond> tradeBookingService;
	PositionService<Bond> positionService;
	RiskService<Bond> riskService;
	GUIService<Bond> guiService;
	InquiryService<Bond> inquiryService;

	// ----- HistoricalDataService initialize -----
	HistoricalDataService<Position<Bond>> historicalPositionService(POSITION);
	HistoricalDataService<PV01<Bond>> historicalRiskService(RISK);
	HistoricalDataService<ExecutionOrder<Bond>> historicalExecutionService(EXECUTION);
	HistoricalDataService<PriceStream<Bond>> historicalStreamingService(STREAMING);
	HistoricalDataService<Inquiry<Bond>> historicalInquiryService(INQUIRY);
	log(LogLevel::INFO, "Trading services initialized.");

	// ----- create listeners -----
	log(LogLevel::INFO, "Linking service listeners...");
	pricingService.AddListener(algoStreamingService.GetAlgoStreamingListener());
	pricingService.AddListener(guiService.GetGUIServiceListener());
	algoStreamingService.AddListener(streamingService.GetStreamingServiceListener());
	marketDataService.AddListener(algoExecutionService.GetAlgoExecutionServiceListener());
	algoExecutionService.AddListener(executionService.GetExecutionServiceListener());
	executionService.AddListener(tradeBookingService.GetTradeBookingServiceListener());
	tradeBookingService.AddListener(positionService.GetPositionListener());
	positionService.AddListener(riskService.GetRiskServiceListener());

	positionService.AddListener(historicalPositionService.GetHistoricalDataServiceListener());
	executionService.AddListener(historicalExecutionService.GetHistoricalDataServiceListener());
	streamingService.AddListener(historicalStreamingService.GetHistoricalDataServiceListener());
	riskService.AddListener(historicalRiskService.GetHistoricalDataServiceListener());
	inquiryService.AddListener(historicalInquiryService.GetHistoricalDataServiceListener());
	log(LogLevel::INFO, "Service listeners linked.");

	// ----- test the data flows -----
	// -- price data -> pricing service -> algo streaming service -> streaming service -> historical data service --
	cout << fixed << setprecision(6);
    log(LogLevel::INFO, "Processing price data...");
	ifstream pricedata(pricePath.c_str());
	pricingService.GetConnector()->Subscribe(pricedata);
	log(LogLevel::INFO, "Price data flows succeed.");

	// -- orderbook data -> market data service -> algo execution service -> execution service -> historical data service --
	log(LogLevel::INFO, "Processing market data...");
	ifstream marketdata(marketDataPath.c_str());
	marketDataService.GetConnector()->Subscribe(marketdata);
	log(LogLevel::INFO, "Market data flows succeed.");

	// -- trade data -> trade booking service -> position service -> risk service -> historical data service --
	log(LogLevel::INFO, "Processing trade data...");
	ifstream tradedata(tradePath.c_str());
	tradeBookingService.GetConnector()->Subscribe(tradedata);
	log(LogLevel::INFO, "Trade data flows succeed.");

	// -- inquiry data -> inquiry service -> historical data service --
	log(LogLevel::INFO, "Processing inquiry data...");
	ifstream inquirydata(inquiryPath.c_str());
	inquiryService.GetConnector()->Subscribe(inquirydata);
	log(LogLevel::INFO, "Inquiry data flows succeed.");
	std::cout << std::endl << std::endl;
	log(LogLevel::FINAL, "Trading system built successfully.");

}
