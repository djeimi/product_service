#ifndef AUTHOR_H
#define AUTHOR_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include <optional>

namespace database
{
    class Product{
        private:
            long _id;
            std::string _name;
            std::string _type;
            std::string _description;
            std::string _price;
            std::string _quantity;
            long _author_id;

        public:

            static Product fromJSON(const std::string & str);

            long             get_id() const;
            const std::string &get_name() const;
            const std::string &get_type() const;
            const std::string &get_description() const;
            const std::string &get_price() const;
            const std::string &get_quantity() const;
            const long &get_author_id() const;

            long&        id();
            std::string &name();
            std::string &type();
            std::string &description();
            std::string &price();
            std::string &quantity();
            long &author_id();

            static void init();
            void update_in_mysql();
            static bool delete_in_mysql(long id);

            static std::optional<Product> read_by_id(long id);
            static std::vector<Product> read_all();
            static std::vector<Product> search(std::string name);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;
    };
}

#endif