#ifndef PROPERTY_H
#define PROPERTY_H
#include <map>
#include <string>

namespace mc {

    class properties {
        std::map<std::string, std::string> _properties;
        std::string _property_filename;
    public:
        properties();
        virtual ~properties();

        void load_properties(const std::string&);
        bool has_property(const std::string&) const;
        const std::string& operator[](const std::string&) const;
        const std::string& get_property(const std::string&) const;
        const std::string& filename() const;
        const std::map<std::string, std::string>& all_properties() const;

        // for adding new properties
        void save();
        void create(const std::string&);
        bool add_property(std::string, std::string);
    private:

    };
    
    class group_properties{
        std::map<std::string, properties> _properties;
        std::string _property_filename;
    public:
        group_properties();
        virtual ~group_properties();
        
        void load_properties(const std::string&);
        bool has_group(const std::string&) const;
        bool has_property(const std::string&, const std::string&) const;
        properties& operator[](const std::string&);
        const properties& operator[](const std::string&) const;
        properties& get_group(const std::string&);
        const properties& get_group(const std::string&) const;
        const std::string& get_property(const std::string&, const std::string&) const;
        const std::string& filename() const;
        const std::map<std::string, properties>& all_properties() const;
        
        void save();
        void add_group(const std::string&);
    };
}

#endif /* PROPERTY_H */