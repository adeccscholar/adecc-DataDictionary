#include "Test_Dictionary.h"

#include <sstream>

TDictionary_Test::TDictionary_Test() : dictionary("simple person model") {
   std::ostringstream os1, os2;

   os1 << "Model for a simple management of personal data for testing and presenting the use of "
       << "metadata with a generator and for discussing the possibilities of using metadata.\n"
       << "The model is implemented using the adecc Scholar metadata class TMyDictionary and "
       << "the associated classes TMyDatatype, TMyAttribute, TMyTable, TMyReferences and TMyIndices.\n"
       << "\\image html ER-Model.jpg\n"
       << "\\image latex ERModelLS.tif";
 
   os2 << "This model was created to show and discuss the potential of metadata and the generation "
       << "of source code, database scripts and documentation.\n"
       << "In the model, in addition to the is - a relationship between the Person and Employee tables, "
       << "there is a part - of relationship to employee working hours and an association to the departments "
       << "in which employees work.\n"
       << "In addition, there are a number of extended value tables to show that these do not only have to "
       << "consist of an ID and name, but can also contain other data.\n"
       << "The project, including all additional files and documentations, are part of the source code of the "
       << "\"adecc Scholar\" project. The purpose of this project is to impart know-how on architecture and "
       << "technology when using the programming language C++. The MIT license applies unless otherwise stated "
       << "or other licenses must be used due to legal requirements.\n"
       << "Finally, please note that these sources are only training materials and explicitly not productive "
       << "sources. You can use them according to the license in your own application, but there is no claim "
       << "to correctness or any form of warranty.";   

   dictionary.Denotation("model with a simple person administration");
   dictionary.Description(os1.str());
   dictionary.Comment(os2.str());


   dictionary.Version("1.0");
   dictionary.Author("Volker Hillmann (adecc Scholar)");
   dictionary.Copyright("adecc Systemhaus GmbH 2024, All rights reserved.");
   dictionary.License("This project is released under the MIT License.");

   dictionary.AddDataType("bigint", "BIGINT", false, false, "", "long long", "", "i", false, "BIGINT is largest integer data type for SQL Server . It uses 8 bytes of storage.");
   dictionary.AddDataType("bool", "TINYINT", false, false, "IN (0, 1)", "bool", "", "bo", false, "boolean (true = 1, false = 2) in source will transformed to TINYINT. This is the smallest integer data type and only uses 1 byte of storage. this data type is an integer value from 0 to 255.");
   dictionary.AddDataType("char", "CHAR", true, false, "", "std::string", "<string>", "c", true, "CHAR is a fixed-sized character data type and will transformed to a std::string. Use this type when values are consistent in length. The max  size is 8,000, storing up to 8,000 ASCII characters.");
   dictionary.AddDataType("date", "DATE", false, false, "", "std::chrono::year_month_day", "<chrono>", "da", false, "date is the DATE data type which is specifies a date in SQL Server.  DATE supports dates from 0001-01-01 through 9999-12-31. Ist will transformed to a std::chrono::year_month_day which is supported from C++20. DATE supports dates from 0001-01-01 through 9999-12-31.");
   dictionary.AddDataType("datetime", "DATE", false, false, "", "std::chrono::system_clock::time_point", "<chrono>", "dt", false, "datetime is the DATETIME data type which specifies a date and time with fractional seconds. Ist supports dates from January 1, 1753, through December 31, 9999. the time is based on 24-hour clock. For source code  this type will interpreted as std::chrono::system_clock::time_point>");
   dictionary.AddDataType("decimal", "DECIMAL", true, true, "", "double", "", "fl", true, "the DECIMAL data type is an exact number with a fixed precision and scale. precision is an integer representing the total number of digits and scale is also an integer value that represents the number of decimal places. It could be transfered to an bcd type, but we use double as representing type for source code.");
   dictionary.AddDataType("double", "FLOAT", false, false, "", "double", "", "fl", true, "The FLOAT data type is an approximate number with floating point data equal to ieee coded values in programming (this means not all values can be represented exactly).");
   dictionary.AddDataType("integer", "INT", false, false, "", "int", "", "i", false, "the INT data type is an the most used integer value type in SQL Server and  uses 4 bytes of storage.  it always stores positive and negative values. The type will translated to an int type of c++.");
   dictionary.AddDataType("smallint", "SMALLINT", false, false, "", "short int", "", "i", false, "the SMALLINT data type is an integer value from -32,768 to 32,767 and uses 2 bytes of storage. In c++ is the short int type used to represent this data.");
   dictionary.AddDataType("unsigned", "INT", false, false, ">= 0", "unsigned int", "", "u", false, "the INT data type is an the most used integer value type in SQL Server and  uses 4 bytes of storage.  it always stores positive and negative values. The type will translated to an int type of c++.");
   dictionary.AddDataType("varchar", "VARCHAR", true, false, "", "std::string", "<string>", "str", true, "the VARCHAR data type stores variable-length character strings and is used when there is variability in the size of the data. The type may hold up to 8,000 ASCII characters of data. For c++ this datatype use std::string.");
   dictionary.AddDataType("text", "VARCHAR(MAX)", false, false, "", "std::string", "<string>", "str", true, "the VARCHAR(MAX) data type stores variable-length character strings and is used to store very large, i.e. max length, character data. It can hold mostly as much as 2GB of ASCII character data. For c++ this datatype use std::string.");


   dictionary.AddTable("Address", EMyEntityType::table, "Address", "dbo", "Address", "myTest", "System", "SQL", "information on the addresses where a person lives, works or has any other relationship with them")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "attribute as Foreign key from an attribute ID of the person entity to whom the address belongs")
      .AddAttribute(2, "AddressType", "AddressType", "integer", 0, 0, true, true, "", "", "extension of the key by the address type to manage different addresses for one person")
      .AddAttribute(3, "Zipcode", "Zipcode", "varchar", 10, 0, true, false, "", "", "Zip code for the address")
      .AddAttribute(4, "City", "City", "varchar", 50, 0, true, false, "", "", "name of the city/county (possibly with district) for this address")
      .AddAttribute(5, "Street", "Street", "varchar", 50, 0, true, false, "", "", "name of the street belonging to this address")
      .AddAttribute(6, "StreetNumber", "StreetNumber", "varchar", 10, 0, false, false, "", "", "house number with addition for this address")
      .AddAttribute(7, "Country", "Country", "integer", 0, 0, true, false, "", "", "ID of the country for the address, key attribute from the \"Countries\" entity")
      
      .AddReference("refAddress2Person", EMyReferenceType::komposition, "Person", "residential address", "part of- relationship from a person to the residential addresses", { {1,1} })
      .AddReference("refAddress2AddressType", EMyReferenceType::range, "AddressTypes", "has values", "range value to extent the relationship of a person to an address", { {2,1} })
      .AddReference("refAddress2Countries", EMyReferenceType::assoziation, "Countries", "belongs to", "key ID to the associated entity of Countries, where this address is located", { {7,1} })

      .AddIndex("idx_Address_City_Street", EMyIndexType::undefined, "access path to search for city / street combinations in addresses", { { 4, true }, { 5, true } });
      ;

   dictionary.AddTable("AddressTypes", EMyEntityType::table, "AddressTypes", "dbo", "AddressTypes", "myTest", "System", "SQL", "domain / range of values for address types, this is an extension of the relationship between persons and addresses.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a record in this domain with address types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "unique description of an entry in the domain, display in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "non-representative abbreviation that can be used for a short advertisement for this address")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "detailed description of these address types, what is associated with them. Self documentation, possible use as hint")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this AddressType, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of address type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_AddressType_Denotation", EMyIndexType::key, "unique / representative denotation for an addresstye as key canditate", { { 3, true } });
      ;

   dictionary.AddTable("Banking", EMyEntityType::table, "Banking", "dbo", "Banking", "myTest", "System", "SQL", "informations about the account details provided by a person and used in a specific context")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "attribute as foreign key from an attribute ID of a person entity to which these account details belong")
      .AddAttribute(2, "BankingType", "BankingType", "integer", 0, 0, true, true, "", "", "extension of the account details key in order to be able to save several / different bank details in a standardized way")
      .AddAttribute(3, "BankName", "BankName", "varchar", 50, 0, false, false, "", "", "name of the bank that manages this account (not necessarily required)")
      .AddAttribute(4, "IBAN", "IBAN", "varchar", 33, 0, true, false, "", "", "IBAN number (account identification) for this bank account (required)")
      .AddAttribute(5, "BIC", "BIC", "varchar", 12, 0, false, false, "", "", "BIC number of the bank that manages this account (not necessarily required for transactions inside europe)")
      .AddAttribute(6, "BankOwner", "BankOwner", "varchar", 27, 0, false, false, "", "", "optionally the name of the owner of the bank details, if this differs from the assigned person")
      .AddAttribute(7, "Country", "Country", "integer", 0, 0, false, false, "", "", "ID of the country for the banking entity, key attribute from the \"Countries\" entity")

      .AddReference("refBanking2Person", EMyReferenceType::komposition, "Person", "owns", "part of relationship from a banking account to the person who own it", { {1,1} })
      .AddReference("refBanking2Type", EMyReferenceType::range, "BankingTypes", "has values", "range value to extent the relationship of a person to an banking account", { {2,1} })
      .AddReference("refBanking2Countries", EMyReferenceType::assoziation, "Countries", "belongs to", "key ID to the associated entity of Countries, where bank who manage this account is located", { {7,1} })

      .AddIndex("idx_Banking_IBAN", EMyIndexType::undefined, "access path to search a specific IBAN account number in data", { { 4, true } })
      .AddIndex("idx_Banking_BIC", EMyIndexType::undefined, "access path to search a specific BIC in data", { { 5, true } })

      ;

   dictionary.AddTable("BankingTypes", EMyEntityType::table, "BankingTypes", "dbo", "BankingTypes", "myTest", "System", "SQL", "domain / range of values for banking types, this is an extension of the relationship between persons and banking accounts.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a record in this domain with banking types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "unique description of an banking type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "non-representative abbreviation that can be used for a short advertisement for this banking type")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "detailed description of these banking type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this type of banking, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of banking type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_BankingTypes_Denotation", EMyIndexType::key, "unique / representative denotation for an banking type as key canditate for this entity", { { 2, true } });

      ;

      dictionary.AddTable("Countries", EMyEntityType::table, "Countries", "dbo", "Countries", "myTest", "System", "SQL", "table with the countries and additional values for the countries. Used by others for assoziation.")
         .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a record in the table Countries")
         .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "unique, official designation of the country, key candidate")
         .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "abbreviation of the country for the program")
         .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "text field for the description of the country. can be used by the program to provide additional information, for example in the status bar or a hint")
         .AddAttribute(5, "CountryDialing", "CountryDialing", "varchar", 10, 0, false, false, "", "", "international country code for telephone for this country (not unique)")
         .AddAttribute(6, "ISO_Code", "ISO_Code", "varchar", 5, 0, false, false, "", "", "unique, official international code (ISO) for the country in the communication")
         .AddAttribute(7, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this country, not used in application")
         .AddAttribute(8, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of country to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_Countries_Denotation", EMyIndexType::key, "unique / representative official name of the country as key canditate for this entity", { { 2, true } })
      .AddIndex("uk_Countries_ISOCode", EMyIndexType::key, "unique / representative official iso code of the country as key canditate for this entity", { { 6, true } })

      ;

   dictionary.AddTable("Departments", EMyEntityType::table, "Departments", "dbo", "Departments", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "")

      .AddIndex("uk_Departments_Denotation", EMyIndexType::key, "unique / representative denotation forthe department as key canditate for this entity", { { 2, true } })
      .AddIndex("idx_Departments_Abbr", EMyIndexType::undefined, "search pass for abbreviations of departments", { { 3, true } })

      ;

   dictionary.AddTable("Employees", EMyEntityType::table, "Employees", "dbo", "Employees", "myHR", "System", "SQL", "information about the employees in the company (generalization of a person)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "attribute as foreign key from an attribute ID of a person entity to which these account details belong")
      .AddAttribute(2, "PersonNumber", "PersonNumber", "varchar", 15, 0, true, false, "", "", "unique HR number of the employee in the company, assigned by the HR department")
      .AddAttribute(3, "Salery", "Salery", "decimal", 9, 2, false, false, ">= 0.0", "", "salary / income that the employee currently receives")
      .AddAttribute(4, "TaxClass", "TaxClass", "integer", 0, 0, true, false, "", "", "Tax class currently held by the employee")
      .AddAttribute(5, "StartOfJob", "StartOfJob", "date", 0, 0, false, false, "", "", "starting date of the employee in the company (can be extended later for the start of the current position)")
      .AddAttribute(6, "JobPosition", "JobPosition", "integer", 0, 0, false, false, "", "", "current position / activity of the employee within the company")
      .AddAttribute(7, "JobSpec", "JobSpec", "varchar", 100, 0, false, false, "", "", "specification of the job as free text to concretize this")
      .AddAttribute(8, "Department", "Department", "integer", 0, 0, true, false, "", "", "id of the department where the employee currently work")
      .AddAttribute(9, "SocialNummer", "SocialNummer", "varchar", 20, 0, true, false, "", "", "social insurance number of the employee")
      .AddAttribute(10, "Active", "Active", "bool", 0, 0, false, false, "", "", "boolean value indicating whether the person is active in the company")

      .AddReference("refEmployees2Person", EMyReferenceType::generalization, "Person", "is-a", "generalization from an employee to a person (is-a relationship)", { {1,1} })
      .AddReference("refEmployees2TaxClass", EMyReferenceType::range, "TaxClasses", "has values", "range value as domain for the attribute TaxClass", { {4,1} })
      .AddReference("refEmployees2JobPositions", EMyReferenceType::range, "JobPositions", "holds", "range value as domain for this attribute, possible jobpositions in company", { {6,1} })
      .AddReference("refEmployees2Department", EMyReferenceType::assoziation, "Departments", "works in", "assoziations between an employee to the department where she/he work", { {8,1} })

      .AddIndex("uk_Employees_PersonNumber", EMyIndexType::key, "unique personal number of a employee in company (key canditate)", { { 2, true } })
      .AddIndex("uk_Employees_SocialNumber", EMyIndexType::key, "extern unique number of an employee (key canditate)", { { 8, true } })
      .AddIndex("idx_Employees_JobSpec", EMyIndexType::undefined, "access path to search for informations about job specifications", { { 6, true } })
      .AddIndex("idx_Employees_Active", EMyIndexType::undefined, "access path to search / filter all active employees", { { 9, true } })

      ;

   dictionary.AddTable("FamilyTypes", EMyEntityType::table, "FamilyTypes", "dbo", "FamilyTypes", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a record in this domain with family types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "unique description of a family type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "non-representative abbreviation that can be used for a short advertisement for this banking type")
      .AddAttribute(4, "IsNaturalPerson", "IsNaturalPerson", "bool", 0, 0, false, false, "", "", "boolean indicator whether this entity is a natural person when true, otherwise ist a legal entity")
      .AddAttribute(5, "Description", "Description", "text", 0, 0, false, false, "", "", "detailed description of these family type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(6, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this type of family status, not used in application")
      .AddAttribute(7, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of family type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_FamilyTypes_Denotation", EMyIndexType::key, "unique / representative denotation for a family type (choosen gender) as key canditate for this entity", { { 2, true } })
      ;

   dictionary.AddTable("FormOfAddress", EMyEntityType::table, "FormOfAddress", "dbo", "FormOfAddress", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "")
      .AddAttribute(5, "TypeSpec", "TypeSpec", "integer", 0, 0, false, false, "", "", "")
      .AddAttribute(6, "Salutation", "Salutation", "varchar", 50, 0, false, false, "", "", "")
      .AddAttribute(7, "Valediction", "Valediction", "varchar", 50, 0, false, false, "", "", "")
      .AddAttribute(8, "Notes", "Notes", "text", 0, 0, false, false, "", "", "")
      .AddAttribute(9, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of form of address type to a system value (cannot be changed as it is used directly by the program)")

      .AddReference("refFormOfAddress2Type", EMyReferenceType::assoziation, "FamilyTypes", "represent", "assoziation between a form of address and a familiy type which is represent", { {5,1} })
      ;

   dictionary.AddTable("Internet", EMyEntityType::table, "Internet", "dbo", "Internet", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
      .AddAttribute(2, "InternetType", "InternetType", "integer", 0, 0, true, true, "", "", "")
      .AddAttribute(3, "Adresse", "Adresse", "varchar", 100, 0, true, false, "", "", "")

      .AddReference("refInternet2Person", EMyReferenceType::komposition, "Person", "owns", "part of relationship from an internet connection to the person who own it", { {1,1} })
      .AddReference("refInternet2Type", EMyReferenceType::range, "InternetTypes", "has values", "range value to extent the relationship of a person to an internet connection", { {2,1} })

      .AddIndex("idxInternet_Address", EMyIndexType::undefined, "access path to seach a address of internet connection", { { 3, true } })
      ;

   dictionary.AddTable("InternetTypes", EMyEntityType::table, "InternetTypes", "dbo", "InternetTypes", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a record in this domain with internet connection types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "unique description of an internet connection type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "non-representative abbreviation that can be used for a short advertisement for this internet connection type")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "detailed description of these internet connection type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Prefix", "Prefix", "varchar", 10, 0, false, false, "", "", "")
      .AddAttribute(6, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this type of internet connection, not used in application")
      .AddAttribute(7, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of internet connection type to a system value (cannot be changed as it is used directly by the program)")

      ;
   
   dictionary.AddTable("JobPositions", EMyEntityType::table, "JobPositions", "dbo", "JobPositions", "myHR", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "")

      ;

   dictionary.AddTable("Person", EMyEntityType::table, "Person", "dbo", "Person", "myTest", "System", "SQL", "informations about a person, base for different kinds of special persons in other areas of the company")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a entity of person")
      .AddAttribute(2, "Name", "Name", "varchar", 30, 0, true, false, "", "", "family name of the natural person respectively the name for a legal entity")
      .AddAttribute(3, "Firstname", "Firstname", "varchar", 30, 0, false, false, "", "", "first name of the natural person respectively a name extenstion for a legal entity")
      .AddAttribute(4, "FormOfAddress", "FormOfAddress", "integer", 0, 0, true, false, "", "", "form of address for this person. this attribute control the kind of a person too")
      .AddAttribute(5, "FamilyStatus", "FamilyStatus", "integer", 0, 0, true, false, "", "", "family status for the person")
      .AddAttribute(6, "FamilyStatusSince", "FamilyStatusSince", "date", 0, 0, false, false, "", "", "the date from which the current family status applies.")
      .AddAttribute(7, "Birthday", "Birthday", "date", 0, 0, false, false, "", "", "birthday of a natural person. This attribute is unused for legal entrities")
      .AddAttribute(8, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this tperson, not used in application")

      .AddReference("refPerson2FormOfAddress", EMyReferenceType::range, "FormOfAddress", "has values", "range value to extent the relationship of a person to a form of address", { {4,1} })
      .AddReference("refPerson2FamilyStatus", EMyReferenceType::range, "FamilyStatus", "has values", "range value to extent the relationship of a person to a family status", { {5,1} })

      .AddIndex("idx_Person_Name_Firstname", EMyIndexType::undefined, "access path to search for name and / or firstname in a person", { { 2, true }, { 3, true } })
      ;

    dictionary.AddTable("Phone", EMyEntityType::table, "Phone", "dbo", "Phone", "myTest", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
       .AddAttribute(2, "PhoneType", "PhoneType", "integer", 0, 0, true, true, "", "", "")
       .AddAttribute(3, "AreaCode", "AreaCode", "varchar", 10, 0, true, false, "", "", "")
       .AddAttribute(4, "CallNumber", "CallNumber", "varchar", 13, 0, true, false, "", "", "")
       .AddAttribute(5, "Country", "Country", "integer", 0, 0, true, false, "", "", "")

       .AddReference("refPhone2Person", EMyReferenceType::komposition, "Person", "owns", "part of relationship from a phone number to the person who own it", { {1,1} })
       .AddReference("refPhone2Type", EMyReferenceType::range, "PhonesTypes", "has values", "range value to extent the relationship of a person to a phone number", { {2,1} })
       .AddReference("refPhone2Countries", EMyReferenceType::assoziation, "Countries", "belongs to", "key ID to the associated entity of Countries, where phone connection located", { {5,1} })

       .AddIndex("idx_Phone_Number", EMyIndexType::undefined, "access path to search a phone number", { { 3, true }, { 4, true } })
       ;

    dictionary.AddTable("PhonesTypes", EMyEntityType::table, "PhoneTypes", "dbo", "PhonesTypes", "myTest", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "unique identification number for a record in this domain with phone types")
       .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "unique description of a phone type entity in the domain, used in selections, comboboxes, … (key canditate)")
       .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "non-representative abbreviation that can be used for a short advertisement for this phone  type")
       .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "detailed description of these phone  type,this is a self documentation, possible use as hint or to information in the programm")
       .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "notes, with additional / free information for this type of banking, not used in application")
       .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of phone type to a system value (cannot be changed as it is used directly by the program)")

       ;

    dictionary.AddTable("WorkingTime", EMyEntityType::table, "WorkingTime", "dbo", "WorkingTime", "myHR", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
       .AddAttribute(2, "StartingTime", "StartingTime", "datetime", 0, 0, true, true, "", "", "")
       .AddAttribute(3, "ClosingTime", "ClosingTime", "datetime", 0, 0, false, false, "", "", "")
       .AddAttribute(4, "Processed ", "Processed ", "bool", 0, 0, true, false, "", "", "")

       .AddReference("refWorkTime2Employee", EMyReferenceType::komposition, "Employees", "worked", "part of relationship of work times to the employees", { {1,1} })
       ;

    dictionary.AddTable("FamilyStatus", EMyEntityType::table, "FamilyStatus", "dbo", "FamilyStatus", "myTest", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
       .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "")
       .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "")
       .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "")
       .AddAttribute(5, "Coupled", "Coupled", "bool", 0, 0, false, false, "", "", "")
       .AddAttribute(6, "NeedDate", "NeedDate", "bool", 0, 0, false, false, "", "", "")
       .AddAttribute(7, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "boolean value that makes this entity of phone type to a system value (cannot be changed as it is used directly by the program)")

       ;


    dictionary.AddTable("TaxClasses", EMyEntityType::table, "TaxClasses", "dbo", "TaxClasses", "myHR", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "")
       .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "")
       .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "")
       .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "")
       .AddAttribute(5, "Coupled", "Coupled", "bool", 0, 0, false, false, "", "", "")
       .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "")

       ;
   }
