// algoexecutionservice.hpp
//
// Purpose: 1. Defines the data types and Service for algo executions.
// 
// @author author Yuanting Li
// @version 1.0 2023/12/23

#ifndef ALGOEXECUTION_SERVICE_HPP
#define ALGOEXECUTION_SERVICE_HPP

#include <string>
#include "soa.hpp"  
#include "marketdataservice.hpp"
#include "utilities.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:

    // ctor for an order
    ExecutionOrder() = default;
    ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, string _parentOrderId, bool _isChildOrder);

    // dtor
    ~ExecutionOrder() = default;

    // Get the product
    const T& GetProduct() const;

    // Get the pricing side
    PricingSide GetSide() const;

    // Get the order ID
    const string& GetOrderId() const;

    // Get the order type on this order
    OrderType GetOrderType() const;

    // Get the price on this order
    double GetPrice() const;

    // Get the visible quantity on this order
    long GetVisibleQuantity() const;

    // Get the hidden quantity
    long GetHiddenQuantity() const;

    // Get the parent order ID
    const string& GetParentOrderId() const;

    // Is child order?
    bool IsChildOrder() const;

    // object printer
    template<typename S>
    friend ostream& operator<<(ostream& output, const ExecutionOrder<S>& order);

private:
    T product;
    PricingSide side;
    string orderId;
    OrderType orderType;
    double price;
    long visibleQuantity;
    long hiddenQuantity;
    string parentOrderId;
    bool isChildOrder;

};

template<typename T>
ExecutionOrder<T>::ExecutionOrder(const T& _product, PricingSide _side, string _orderId, OrderType _orderType,
    double _price, double _visibleQuantity, double _hiddenQuantity,
    string _parentOrderId, bool _isChildOrder)
    : product(_product), side(_side), orderId(move(_orderId)), orderType(_orderType), price(_price),
    visibleQuantity(_visibleQuantity), hiddenQuantity(_hiddenQuantity), parentOrderId(move(_parentOrderId)),
    isChildOrder(_isChildOrder)
{
}

template<typename T>
const T& ExecutionOrder<T>::GetProduct() const
{
    return product;
}

template<typename T>
PricingSide ExecutionOrder<T>::GetSide() const
{
    return side;
}


template<typename T>
const string& ExecutionOrder<T>::GetOrderId() const
{
    return orderId;
}

template<typename T>
OrderType ExecutionOrder<T>::GetOrderType() const
{
    return orderType;
}

template<typename T>
double ExecutionOrder<T>::GetPrice() const
{
    return price;
}

template<typename T>
long ExecutionOrder<T>::GetVisibleQuantity() const
{
    return visibleQuantity;
}

template<typename T>
long ExecutionOrder<T>::GetHiddenQuantity() const
{
    return hiddenQuantity;
}

template<typename T>
const string& ExecutionOrder<T>::GetParentOrderId() const
{
    return parentOrderId;
}

template<typename T>
bool ExecutionOrder<T>::IsChildOrder() const
{
    return isChildOrder;
}

template<typename T>
ostream& operator<<(ostream& output, const ExecutionOrder<T>& order)
{
    string productId = order.GetProduct().GetProductId();
    string orderId = order.GetOrderId();
    string side = (order.GetSide() == BID) ? "Bid" : "Ask";

    string orderType;
    switch (order.GetOrderType())
    {
    case FOK: orderType = "FOK"; break;
    case MARKET: orderType = "MARKET"; break;
    case LIMIT: orderType = "LIMIT"; break;
    case STOP: orderType = "STOP"; break;
    case IOC: orderType = "IOC"; break;
    }

    string price = Price2Frac(order.GetPrice());
    string visibleQuantity = to_string(order.GetVisibleQuantity());
    string hiddenQuantity = to_string(order.GetHiddenQuantity());
    string parentOrderId = order.GetParentOrderId();
    string isChildOrder = order.IsChildOrder() ? "True" : "False";

    output << productId << "," << orderId << "," << side << "," << orderType << ","
        << price << "," << visibleQuantity << "," << hiddenQuantity << ","
        << parentOrderId << "," << isChildOrder;

    return output;
}

/**
 * Algo Execution Service to execute orders on market.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class AlgoExecution
{
private:
    ExecutionOrder<T> executionOrder;
    Market market;

public:
    // ctor for an order
    AlgoExecution() = default; // needed for map data structure later
    AlgoExecution(const ExecutionOrder<T> &_executionOrder, Market _market);

    // Get the execution order
    const ExecutionOrder<T>& GetExecutionOrder() const;

    // Get the market
    Market GetMarket() const;

};    

template<typename T>
AlgoExecution<T>::AlgoExecution(const ExecutionOrder<T> &_executionOrder, Market _market) :
  executionOrder(_executionOrder), market(_market)
{
}

template<typename T>
const ExecutionOrder<T>& AlgoExecution<T>::GetExecutionOrder() const
{
  return executionOrder;
}

template<typename T>
Market AlgoExecution<T>::GetMarket() const
{
  return market;
}

// forward declaration of AlgoExecutionServiceListener
template<typename T>
class AlgoExecutionServiceListener;


/**
 * Algo Execution Service to execute orders on market.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class AlgoExecutionService : public Service<string, AlgoExecution<T>>
{
private:
  map<string, AlgoExecution<T>> algoExecutionData; // store algo execution data keyed by product identifier
  vector<ServiceListener<AlgoExecution<T>>*> listeners; // list of listeners to this service
  AlgoExecutionServiceListener<T>* algoexecservicelistener;
  double spread;
  long count;

public:
    // ctor
    AlgoExecutionService();
    // dtor
    ~AlgoExecutionService() = default;
    
    // Get data on our service given a key
    AlgoExecution<T>& GetData(string key);
    
    // The callback that a Connector should invoke for any new or updated data
    void OnMessage(AlgoExecution<T>& data) override;
    
    // Add a listener to the Service for callbacks on add, remove, and update events for data to the Service
    void AddListener(ServiceListener<AlgoExecution<T>> *listener) override;
    
    // Get all listeners on the Service.
    const vector< ServiceListener<AlgoExecution<T>>* >& GetListeners() const override;
    
    // Get the special listener for algo execution service
    AlgoExecutionServiceListener<T>* GetAlgoExecutionServiceListener();

    // Execute an algo order on a market, called by AlgoExecutionServiceListener to subscribe data from Algo Market Data Service to Algo Execution Service
    void AlgoExecuteOrder(OrderBook<T>& _orderBook);
    
};

template<typename T>
AlgoExecutionService<T>::AlgoExecutionService() : algoexecservicelistener(new AlgoExecutionServiceListener<T>(this)), count(0)
{
}

template<typename T>
AlgoExecution<T>& AlgoExecutionService<T>::GetData(string key)
{
    auto it = algoExecutionData.find(key);
    if (it != algoExecutionData.end())
    {
        return it->second;
    }
    else
    {
        throw std::runtime_error("Key not found");
    }
}

/**
 * OnMessage() used to be called by input connector to subscribe data
 * no need to implement here.
 */
template<typename T>
void AlgoExecutionService<T>::OnMessage(AlgoExecution<T>& data)
{
}

template<typename T>
void AlgoExecutionService<T>::AddListener(ServiceListener<AlgoExecution<T>> *listener)
{
    listeners.push_back(listener);
}

template<typename T>
const vector< ServiceListener<AlgoExecution<T>>* >& AlgoExecutionService<T>::GetListeners() const
{
    return listeners;
}

template<typename T>
AlgoExecutionServiceListener<T>* AlgoExecutionService<T>::GetAlgoExecutionServiceListener()
{
    return algoexecservicelistener;
}

/**
 * Similar to AddExecutionOrder in executionservice.hpp
 * Execute an algo order on a market, called by AlgoExecutionServiceListener to subscribe data from Algo Market Data Service to Algo Execution Service
 * 1. Store the listened market orderbook data into algo execution map
 * 2. Flow the data to listeners
 */
template<typename T>
void AlgoExecutionService<T>::AlgoExecuteOrder(OrderBook<T>& _orderBook)
{
    // get the order book data
    T product = _orderBook.GetProduct();
    string key = product.GetProductId();
    string orderId = "Algo" + GenerateRandomId(11);
    string parentOrderId = "AlgoParent" + GenerateRandomId(5);

    // get the best bid and offer order and their corresponding price and quantity
    BidOffer bidOffer = _orderBook.BestBidOffer();
    Order bid = bidOffer.GetBidOrder();
    Order offer = bidOffer.GetOfferOrder();
    double bidPrice = bid.GetPrice();
    double offerPrice = offer.GetPrice();
    long bidQuantity = bid.GetQuantity();
    long offerQuantity = offer.GetQuantity();

    PricingSide side;
    double price;
    long quantity;
    // only agressing when the spread is at its tightest (1/128)
    if (offerPrice - bidPrice <= 1.0 / 128.0) {
        // alternating between bid and offer 
        // taking the opposite side of the book to cross the spread, i.e., market order
        if (count % 2 == 0) {
            side = BID;
            price = offerPrice; // BUY order takes best ask price
            quantity = bidQuantity;
        }
        else {
            side = OFFER;
            price = bidPrice; // SELL order takes best bid price
            quantity = offerQuantity;
        }
    }

    // update the count
    count++;

    // Create the execution order
    long visibleQuantity = quantity;
    long hiddenQuantity = 0;
    bool isChildOrder = false;
    OrderType orderType = MARKET; // market order
    ExecutionOrder<T> executionOrder(product, side, orderId, orderType, price, visibleQuantity, hiddenQuantity, parentOrderId, isChildOrder);

    // Create the algo execution
    Market market = BROKERTEC;
    AlgoExecution<T> algoExecution(executionOrder, market);

    // update the algo execution map
    if (algoExecutionData.find(key) != algoExecutionData.end()) 
    { 
        algoExecutionData.erase(key); 
    }
    algoExecutionData.insert(pair<string, AlgoExecution<T>>(key, algoExecution));

    // flow the data to listeners
    for (auto& l : listeners) {
        l->ProcessAdd(algoExecution);
    }
}


/**
* Algo Execution Service Listener subscribing data from Market Data Service to Algo Execution Service.
* Type T is the product type.
*/
template<typename T>
class AlgoExecutionServiceListener : public ServiceListener<OrderBook<T>>
{
private:
  AlgoExecutionService<T>* service;

public:
    // ctor
    AlgoExecutionServiceListener(AlgoExecutionService<T>* _service);
    // dtor
    ~AlgoExecutionServiceListener()=default;
    
    // Listener callback to process an add event to the Service
    void ProcessAdd(OrderBook<T> &data) override;
    
    // Listener callback to process a remove event to the Service
    void ProcessRemove(OrderBook<T> &data) override;
    
    // Listener callback to process an update event to the Service
    void ProcessUpdate(OrderBook<T> &data) override;
    
};

template<typename T>
AlgoExecutionServiceListener<T>::AlgoExecutionServiceListener(AlgoExecutionService<T>* _service)
{
  service = _service;
}

/**
 * ProcessAdd() method is used by listener to subscribe data from Market Data Service to Algo Execution Service.
 * It calls AlgoExecuteOrder() method, change the data type from OrderBook<T> to AlgoExecution<T> and notify the listeners.
 */
template<typename T>
void AlgoExecutionServiceListener<T>::ProcessAdd(OrderBook<T> &data)
{
  service->AlgoExecuteOrder(data);
}

template<typename T>
void AlgoExecutionServiceListener<T>::ProcessRemove(OrderBook<T> &data)
{
}

template<typename T>
void AlgoExecutionServiceListener<T>::ProcessUpdate(OrderBook<T> &data)
{
}


#endif