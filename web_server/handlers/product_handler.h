#ifndef USEHANDLER_H
#define USEHANDLER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/product.h"

static bool hasSubstr(const std::string &str, const std::string &substr)
{
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i)
    {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

class ProductHandler : public HTTPRequestHandler
{
private:
    bool check_name(const std::string &name, std::string &reason)
    {
        if (name.length() < 3)
        {
            reason = "Name must be at leas 3 signs";
            return false;
        }

        return true;
    };

    bool check_price(const std::string &price, std::string &reason)
    {
        if (stod(price) < 0)
        {
            reason = "Price must be non-negative";
            return false;
        }

        return true;
    }

    void badRequestError(HTTPServerResponse &response,  std::string instance)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/bad_request");
        root->set("title", "Internal exception");
        root->set("status", "400");
        root->set("detail", "Недостаточно параметров в теле запроса");
        root->set("instance", instance);
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

    void notFoundError(HTTPServerResponse &response, std::string instance, std::string message)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/not_found");
        root->set("title", "Internal exception");
        root->set("status", "404");
        root->set("detail", message);
        root->set("instance", instance);
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    };

public:
    ProductHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        try
        {
            if (hasSubstr(request.getURI(), "/product"))
            {
                if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
                {
                    if(form.has("id"))
                    {
                        long id = atol(form.get("id").c_str());

                        std::optional<database::Product> result = database::Product::read_by_id(id);
                        if (result)
                        {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            std::ostream &ostr = response.send();
                            Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), "product not found");
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                    
                }
                else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
                {
                    if (form.has("name") && form.has("type") && form.has("desc") &&
                     form.has("price") && form.has("author_id"))
                    {
                        database::Product product;
                        product.name() = form.get("name");
                        product.type() = form.get("type");
                        product.description() = form.get("description");
                        product.price() = form.get("price");
                        product.quantity() = form.get("quantity");
                        //add check foreign key
                        product.author_id() = atol(form.get("author_id").c_str());
    
                        bool check_result = true;
                        std::string message;
                        std::string reason;
    
                        if (!check_name(product.get_name(), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
                        if(!check_price(product.get_price(), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
    
                        if (check_result)
                        {
                            product.save_to_mysql();
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            std::ostream &ostr = response.send();
                            ostr << product.get_id();
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), message);
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                }
                else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_PUT)
                {
                    if (form.has("id") && form.has("name") && form.has("type") &&
                     form.has("description") && form.has("price") && form.has("quantity") && form.has("author_id"))
                    {
                        database::Product product;
                        product.id() = atol(form.get("id").c_str());
                        product.name() = form.get("name");
                        product.type() = form.get("type");
                        product.description() = form.get("description");
                        product.price() = form.get("price");
                        product.quantity() = form.get("quantity");
                        //add check foreign key
                        product.author_id() = atol(form.get("author_id").c_str());
    
                        bool check_result = true;
                        std::string message;
                        std::string reason;
    
                        if (!check_name(product.get_name(), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
                        if(!check_price(product.get_price(), reason))
                        {
                            check_result = false;
                            message += reason;
                            message += "<br>";
                        }
    
                        if (check_result)
                        {
                            product.update_in_mysql();
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), message);
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                }
                else if(request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
                {
                    if(form.has("id"))
                    {
                        long id = atol(form.get("id").c_str());

                        if (database::Product::delete_in_mysql(id))
                        {
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            return;
                        }
                        else
                        {
                            notFoundError(response, request.getURI(), "product not found");
                            return;
                        }
                    }
                    else
                    {
                        badRequestError(response, request.getURI());
                        return;
                    }
                    
                }
                
            }
            else if (hasSubstr(request.getURI(), "/all_products"))
            {
                auto results = database::Product::read_all();
                if(!results.empty())
                {
                    Poco::JSON::Array arr;
                    for (auto s : results)
                        arr.add(s.toJSON());

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(arr, ostr);
                    return;
                    
                }
                else
                {
                    notFoundError(response, request.getURI(), "product not found");
                    return;
                }                            
            }
            else if (hasSubstr(request.getURI(), "/search_products"))
            {
                if(form.has("name") )
                {
                    std::string fn = form.get("name");
                    auto results = database::Product::search(fn);
                    if(!results.empty())
                    {
                        Poco::JSON::Array arr;
                        for (auto s : results)
                            arr.add(s.toJSON());
                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(arr, ostr);
                        return;
                    }
                    else
                    {
                        notFoundError(response, request.getURI(), "product not found");
                        return;
                    }                   
                }
                else
                {
                    badRequestError(response, request.getURI());
                    return;
                }
            }          
        }
        catch (...)
        {
        }
        notFoundError(response, request.getURI(), "request ot found");
    }

private:
    std::string _format;
};
#endif