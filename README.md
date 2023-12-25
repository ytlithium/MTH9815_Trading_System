# MTH9815 Trading System

## Introduction

This bond trading system is specifically designed for US Treasury securities, focusing on 2Y, 3Y, 5Y, 7Y, 10Y, 20Y, and 30Y bonds. The system is structured around a service-oriented architecture, leveraging specialized services and connectors for efficient data flow and processing.

## System Overview

The trading system is built on a robust Service-Oriented Architecture (SOA), leveraging a range of services and connectors to handle different aspects of bond trading. This architecture ensures modularity, scalability, and ease of maintenance.

### Key Components:

## Detailed System Components

### Bond-Specific Services

- **BondTradeBookingService**: Manages trade booking activities. It reads data from `trades.txt` and alternates trades between BUY and SELL for each security across different books (TRSY1, TRSY2, TRSY3).
- **BondPositionService**: Calculates and tracks positions based on trades. It receives data from `BondTradeBookingService` via a `ServiceListener`.
- **BondRiskService**: Assesses and manages the risk associated with bond positions. It is linked to `BondPositionService` and updates risk metrics based on position changes.
- **BondPricingService**: Provides real-time pricing data for bonds, sourced from `prices.txt`. The service updates bond prices, oscillating between specified ranges, and reflects these changes in the system.
- **BondMarketDataService**: Maintains and updates the order book for each bond. It processes data from `marketdata.txt`, ensuring accurate representation of market conditions.
- **BondExecutionService**: Handles the execution of bond trades. It interacts with `BondAlgoExecutionService` for decision-making based on market data.
- **BondStreamingService**: Manages streaming of bond prices and related information, using data from `BondAlgoStreamingService`.
- **BondInquiryService**: Processes inquiries related to bond trades. It reads from `inquiries.txt`, responds to inquiries, and updates their status.
- **BondHistoricalDataService**: Records historical data for various aspects of the trading system, including positions, risks, executions, and streams.
- **BondAlgoExecutionService**: Automates the execution process by analyzing market data and deciding on the optimal timing and price for trade executions.
- **BondAlgoStreamingService**: Facilitates automated streaming of bond prices, making decisions based on the current pricing data.

### GUI Service

- **GUIService**: Provides a graphical interface for monitoring real-time bond price updates. It is throttled to manage data flow and prevent overload.

### Data Handling and Formats

- **Fractional Notation**: Bond prices are expressed in fractional notation, with precision up to 1/256th.
- **Timestamps**: All output files feature timestamps with millisecond precision for accurate record-keeping.

### IO Files

- **Data Files**: The system interacts with various data files like `prices.txt`, `trades.txt`, `marketdata.txt`, and `inquiries.txt`, each serving a specific purpose in the trading workflow.
- **Output Files**: These include files like `positions.txt`, `risk.txt`, `executions.txt`, `streaming.txt`, and `allinquiries.txt`, which store historical data and other relevant information.

## Installation

### Prerequisites:

- UNIX environment
- g++ version 7.x or greater
- Makefile and CMake for compilation

### Compilation:

1. Clone the repository to your local machine.
2. Navigate to the project directory.
3. Use CMake to set up the Makefile.
4. Run `make` to compile the code.

## Usage

1. Start the individual services by running the corresponding executables.
2. Update market data by service communications. 

## Contribution

This system has been developed as a part of the MTH 9815: Software Engineering for Finance course. Contributions and improvements are welcome, adhering to the coding standards and architectural design.
