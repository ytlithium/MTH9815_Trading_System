// pricingservice.hpp
//
// Purpose: 1. Defines the data types and Service for internal positions.
// 2. Design PositionService and PositionServiceListener for position data query and updating.
// 
// @author Breman Thuraisingham
// @coauthor Yuanting Li
// @version 2.0 2023/12/22 

#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

	// ctor
	Position() = default;

	// ctor for a position
	Position(const T& _product);

	// dtor
	~Position() = default;

	// Get the product
	const T& GetProduct() const;

	// Get the position quantity
	long GetPosition(string &book);

	// Get the aggregate position
	long GetAggregatePosition();

	//  send position to risk service through listener
	void AddPosition(string &book, long position);

	// object printer
	template<typename S>
	friend ostream& operator<<(ostream& output, const Position<S>& position);

private:
  T product;
  map<string,long> bookPositionData;

};


template<typename T>
Position<T>::Position(const T &productId) : product(productId)
{
}

template<typename T>
const T& Position<T>::GetProduct() const
{
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book)
{
  return bookPositionData[book];
}

template<typename T>
long Position<T>::GetAggregatePosition()
{
  long sum = 0;
  for (auto it = bookPositionData.begin(); it != bookPositionData.end(); ++it)
  {
    sum += it->second;
  }
  return sum;
}

template<typename T>
void Position<T>::AddPosition(string &book, long position)
{
	if (bookPositionData.find(book) == bookPositionData.end())
	{
		bookPositionData.insert(pair<string,long>(book,position));
	}
	else
	{
		bookPositionData[book] += position;
	}
}

template<typename T>
ostream& operator<<(ostream& output, const Position<T>& position)
{
	T product = position.GetProduct();
	string productId = product.GetProductId();

	output << productId;
	for (const auto& bookPositionPair : position.bookPositionData)
	{
		output << "," << bookPositionPair.first << "," << bookPositionPair.second;
	}

	return output;
}

// Pre-declaration of a listener used to subscribe data from trade booking service
template<typename T>
class PositionServiceListener;

/**
 * Position Service to manage positions across multiple books and securities.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string,Position <T> >
{
private:
  map<string,Position<T>> positionData;
  vector<ServiceListener<Position<T>>*> listeners;
  PositionServiceListener<T>* positionlistener;

public:
  // ctor and dtor
  PositionService();
  ~PositionService()=default;

  // Get data on our service given a key
  Position<T>& GetData(string key);

  // The callback that a Connector should invoke for any new or updated data
  void OnMessage(Position<T> &data);

  // Add a listener to the Service for callbacks on add, remove, and update events
  // for data to the Service.
  void AddListener(ServiceListener<Position<T>> *listener);

  // Get all listeners on the Service.
  const vector<ServiceListener<Position<T>>*>& GetListeners() const;

  // Get the special listener for trade booking service
  PositionServiceListener<T>* GetPositionListener();

  // Add a trade to the service
  void AddTrade(const Trade<T> &trade);

};

template<typename T>
PositionService<T>::PositionService()
{
  positionlistener = new PositionServiceListener<T>(this);
}

template<typename T>
Position<T>& PositionService<T>::GetData(string key)
{
	auto it = positionData.find(key);
	if (it != positionData.end())
	{
		return it->second;
	}
	else
	{
		throw std::runtime_error("Key not found");
	}
}

/**
 * OnMessage() used to be called by an input connector to subscribe data from socket
 * no need to implement here.
 */
template<typename T>
void PositionService<T>::OnMessage(Position<T> &data)
{
}

template<typename T>
void PositionService<T>::AddListener(ServiceListener<Position<T>> *listener)
{
  listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<Position<T>>*>& PositionService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
PositionServiceListener<T>* PositionService<T>::GetPositionListener()
{
  return positionlistener;
}

/**
 * AddTrade() method is used to subscribe data from trade booking service,
 * transfer Trade<T> data to Position<T> data and save it to the service.
 */
template<typename T>
void PositionService<T>::AddTrade(const Trade<T> &trade)
{
  T product = trade.GetProduct();
  string productId = product.GetProductId();
  string book = trade.GetBook();
  long quantity = (trade.GetSide() == BUY) ? trade.GetQuantity() : -trade.GetQuantity();
  if (positionData.find(productId) == positionData.end())
  {
    Position<T> position(product);
    position.AddPosition(book,quantity);
    positionData.insert(pair<string,Position<T>>(productId,position));
  }
  else
  {
    positionData[productId].AddPosition(book,quantity);
  }
  for (auto& listener: listeners)
  {
    listener->ProcessAdd(positionData[productId]);
  }

}

/**
 * Listener class for PositionService.
 * Used to subscribe data from trade booking service instead of connector.
 * Type T is the product type.
 */
template<typename T>
class PositionServiceListener : public ServiceListener<Trade<T>>
{
private:
  PositionService<T>* positionservice;

public:
  // ctor
  PositionServiceListener(PositionService<T>* _positionservice);

  // Listener callback to process an add event to the Service
  void ProcessAdd(Trade<T> &data);

  // Listener callback to process a remove event to the Service
  void ProcessRemove(Trade<T> &data);

  // Listener callback to process an update event to the Service
  void ProcessUpdate(Trade<T> &data);

};

template<typename T>
PositionServiceListener<T>::PositionServiceListener(PositionService<T>* _positionservice)
{
  positionservice = _positionservice;
}

/**
 * ProcessAdd() method is used to subscribe data from trade booking service,
 * and then call AddTrade() method to add the trade to the service.
 */
template<typename T>
void PositionServiceListener<T>::ProcessAdd(Trade<T> &data)
{
  positionservice->AddTrade(data);
}

template<typename T>
void PositionServiceListener<T>::ProcessRemove(Trade<T> &data)
{
}

template<typename T>
void PositionServiceListener<T>::ProcessUpdate(Trade<T> &data)
{
}



#endif
