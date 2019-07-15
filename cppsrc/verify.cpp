#include "verify.h"

bool JWT::verify(std::string token)
{
  try
  {
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify().allow_algorithm(jwt::algorithm::hs256{"ThisIsASuperSecretSecret"});
    verifier.verify(decoded);
    return true;
  }
  catch(const std::exception &e)
  {
    auto p = std::current_exception();
    std::cout << "Problem decoding.." << std::endl;
    return false;
  }
}