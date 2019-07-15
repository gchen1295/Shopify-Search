#ifndef _PRODUCT_H_
#define _PRODUCT_H_
#include <nan.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <boost/algorithm/string.hpp>

class Variant
{
public:
  Variant(std::string id, std::string title, std::string sku, std::string price, std::string available);
  std::string getID();
  std::string getTitle();
  std::string getSKU();
  std::string getPrice();
  std::string getAvailable();
  bool isAvailable();

private:
  std::string id;
  std::string title;
  std::string sku;
  std::string price;
  std::string available;
};

class Product
{
public:
  Product(std::string id, std::string title, std::string handle, std::string image);
  ~Product();
  void addVariant(std::string id, std::string title, std::string sku, std::string price, std::string available);
  void removeVariant(std::string id);
  void printFields();
  std::string getProduct();
  std::string getID();
  std::string getTitle();
  std::string getHandle();
  std::string getVariants();
  std::string getImage();///

private:
  std::string id;
  std::string title;
  std::string handle;
  std::string image;
  //std::unordered_map<std::string, Variant *> variants;
  std::map<std::string, Variant *> variants;
};

class ProductCollection
{
public:
  void createProduct(std::string id, std::string title, std::string handle, std::string image);
  void addVariant(std::string productID, std::string id, std::string title, std::string sku, std::string price, std::string available);
  void removeProduct(std::string id);
  std::string getProducts();
  void printAll();
  Product *findProductByID(std::string id);
  Product *findProductByTitle(std::string title);
  Product *findProductByHandle(std::string title);

private:
  std::unordered_map<std::string, Product *> products;
  //std::map<std::string, Product *> products;
};

#endif