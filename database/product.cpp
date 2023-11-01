#include "product.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void Product::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Product` ("
                        << "`product_id` INT NOT NULL AUTO_INCREMENT,"
                        << "`name` VARCHAR(256) NOT NULL,"
                        << "`type` VARCHAR(256) NOT NULL,"
                        << "`description` VARCHAR(256) NOT NULL,"
                        << "`price` INT NOT NULL,"
                        << "`quantity` VARCHAR(256) NOT NULL,"
                        << "`author_id` INT NOT NULL,"
                        << "PRIMARY KEY (`product_id`),"
                        << "FOREIGN KEY (`author_id`) REFERENCES `user` (`user_id`)"
                        << ");",
                now;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Product::update_in_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement update(session);

            update  << "UPDATE `Product`"
                    << "SET `name` = ?, `type` = ?, `description` = ?, `price` = ?, `price` = ?, `author_id`= ? "
                    << "where `service_id` = ?",
                use(_name),
                use(_type),
                use(_description),
                use(_price),
                use(_quantity),
                use(_author_id),
                use(_id);

            update.execute();

            std::cout << "updated: " << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    bool Product::delete_in_mysql(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement del(session);
            
            Product a;
            del << "DELETE FROM `Product` WHERE `product_id` = ?;",
                use(id),
                range(0, 1); 
            del.execute();
            std::cout << "deleted: " << id << std::endl;
            return true;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
        return false;
    }

    Poco::JSON::Object::Ptr Product::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("name", _name);
        root->set("type", _type);
        root->set("description", _description);
        root->set("price", _price);
        root->set("quantity", _quantity);
        root->set("author_id", _author_id);

        return root;
    }

    Product Product::fromJSON(const std::string &str)
    {
        Product product;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        product.id() = object->getValue<long>("id");
        product.name() = object->getValue<std::string>("name");
        product.type() = object->getValue<std::string>("type");
        product.description() = object->getValue<std::string>("description");
        product.price() = object->getValue<std::string>("price");
        product.quantity() = object->getValue<std::string>("quantity");
        product.author_id() = object->getValue<long>("author_id");

        return product;
    }

    std::optional<Product> Product::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Product a;
            select << "SELECT id, name, type, description, price, quantity, price, author_id FROM Product where id=?",
                into(a._id),
                into(a._name),
                into(a._type),
                into(a._description),
                into(a._price),
                into(a._quantity),
                into(a._author_id),
                use(id),
                range(0, 1); //  iterate over result set one row at a time

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            
        }
        return {};
    }

    std::vector<Product> Product::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Product> result;
            Product a;
            select << "SELECT id, name, type, description, price, quantity, price, author_id FROM Product",
                into(a._id),
                into(a._name),
                into(a._type),
                into(a._description),
                into(a._price),
                into(a._quantity),
                into(a._author_id),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Product> Product::search(std::string name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Product> result;
            Product a;
            name += "%";
            select << "SELECT id, name, type, description, price, quantity, price, author_id FROM Product where name LIKE ?",
                into(a._id),
                into(a._name),
                into(a._type),
                into(a._description),
                into(a._price),
                into(a._quantity),
                into(a._author_id),
                use(name),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if (select.execute())
                    result.push_back(a);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Product::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Product (name, type, description, price, quantity, author_id) VALUES(?, ?, ?, ?, ?, ?)",
                use(_name),
                use(_type),
                use(_description),
                use(_price),
                use(_quantity),
                use(_author_id);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    const long &Product::get_author_id() const
    {
        return _author_id;
    }

    long &Product::author_id()
    {
        return _author_id;
    }

    long Product::get_id() const
    {
        return _id;
    }

    const std::string &Product::get_name() const
    {
        return _name;
    }

    const std::string &Product::get_type() const
    {
        return _type;
    }

    const std::string &Product::get_description() const
    {
        return _description;
    }

    const std::string &Product::get_price() const
    {
        return _price;
    }

    long &Product::id()
    {
        return _id;
    }

    std::string &Product::name()
    {
        return _name;
    }

    std::string &Product::type()
    {
        return _type;
    }

    std::string &Product::description()
    {
        return _description;
    }

    std::string &Product::price()
    {
        return _price;
    }

    std::string &Product::quantity()
    {
        return _quantity;
    }
}