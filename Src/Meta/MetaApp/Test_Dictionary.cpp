#include "Test_Dictionary.h"

#include <sstream>

TDictionary_Test::TDictionary_Test() : dictionary("simple person model") {
   std::ostringstream os1, os2;

   os1 << "Model for a simple management of personal data for testing and presenting the use of "
       << "metadata with a generator and for discussing the possibilities of using metadata.\n"
       << "The model is implemented using the adecc Scholar metadata class TMyDictionary and "
       << "the associated classes TMyDatatype, TMyAttribute, TMyTable, TMyReferences and TMyIndices.";
 
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



   dictionary.AddDataType("bigint", "BIGINT", false, false, "long long", "", "i", "0", false, "große Ganzzahl");
   dictionary.AddDataType("bool", "SMALLINT", false, false, "bool", "", "bo", "false", false, "");
   dictionary.AddDataType("char", "CHAR", true, false, "std::string", "<string>", "c", "' '", true, "Zeichenkette mit fester Länge");
   dictionary.AddDataType("date", "DATE", false, false, "std::chrono::year_month_day", "<chrono>", "da", "std::chrono::system_clock::now()", false, "");
   dictionary.AddDataType("datetime", "DATE", false, false, "std::chrono::system_clock::time_point>", "<chrono>", "dt", "std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())", false, "");
   dictionary.AddDataType("decimal", "DECIMAL", true, true, "double", "", "fl", "0.0", true, "binary coded decimal");
   dictionary.AddDataType("double", "FLOAT", false, false, "double", "", "fl", "0.0", true, "");
   dictionary.AddDataType("integer", "INT", false, false, "int", "", "i", "0", false, "");
   dictionary.AddDataType("smallint", "SMALLINT", false, false, "int", "", "i", "0", false, "normalerweise als bool interpretierter SmallInt- Type");
   dictionary.AddDataType("varchar", "VARCHAR", true, false, "std::string", "<string>", "str", "\"\"", true, "Zeichenkette mit variabler Länge");
   dictionary.AddDataType("text", "VARCHAR(MAX)", false, false, "std::string", "<string>", "str", "", true, "Text, Zeichenkette mit maximaler Länge");


   dictionary.AddTable("Address", EMyEntityType::table, "Address", "dbo", "Address", "myTest", "System", "SQL", "information on the addresses where a person lives, works or has any other relationship with them")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "attribute as Foreign key from the person to whom the address belongs")
      .AddAttribute(2, "AddressType", "AddressType", "integer", 0, 0, true, true, "extension of the key by the address type to manage different addresses for one person")
      .AddAttribute(3, "Street", "Street", "varchar", 50, 0, true, false, "name of the street belonging to this address")
      .AddAttribute(4, "StreetNumber", "StreetNumber", "varchar", 10, 0, false, false, "house number with addition for this address")
      .AddAttribute(5, "City", "City", "varchar", 50, 0, true, false, "name of the city/county (possibly with district) for this address")
      .AddAttribute(6, "Zipcode", "Zipcode", "varchar", 10, 0, true, false, "Zip code for the address")
      .AddAttribute(7, "Country", "Country", "integer", 0, 0, true, false, "ID of the country for the address, key attribute from the \"Countries\" entity")

      .AddReference("refAddress2Person", EMyReferenceType::komposition, "Person", "part of relationship from a address to the person who own it", { {1,1} })
      .AddReference("refAddress2AddressType", EMyReferenceType::range, "AddressTypes", "range value to extent the relationship of a person to an address", { {2,1} })
      .AddReference("refAddress2Countries", EMyReferenceType::assoziation, "Countries", "key ID to the associated entity of Countries, where this address is located", { {7,1} })

      .AddIndex("idx_Address_City_Street", EMyIndexType::undefined, "access path to search for city / street combinations in addresses", { { 4, true }, { 5, true } });
      ;

   dictionary.AddTable("AddressTypes", EMyEntityType::table, "AddressTypes", "dbo", "AddressTypes", "myTest", "System", "SQL", "domain / range of values for address types, this is an extension of the relationship between persons and addresses.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "unique identification number for a record in this domain")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "unique description of an entry in the domain, display in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "non-representative abbreviation that can be used for a short advertisement for this address")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "detailed description of these address types, what is associated with them. Self documentation, possible use as hint")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "notes, with additional / free information for this AddressType, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "boolean value that makes this type a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_AddressType_Denotation", EMyIndexType::key, "unique / representative denotation for an addresstye as key canditate", { { 3, true } });
      ;

   dictionary.AddTable("Banking", EMyEntityType::table, "Banking", "dbo", "Banking", "myTest", "System", "SQL", "informations about the account details provided by a person and used in a specific context")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "attribute as foreign key of a person entity to which these account details belong")
      .AddAttribute(2, "BankingType", "BankingType", "integer", 0, 0, true, true, "extension of the account details key in order to be able to save several / different bank details in a standardized way")
      .AddAttribute(3, "BankName", "BankName", "varchar", 50, 0, false, false, "name of the bank that manages this account (not necessarily required)")
      .AddAttribute(4, "IBAN", "IBAN", "varchar", 33, 0, true, false, "IBAN number (account identification) for this bank account (required)")
      .AddAttribute(5, "BIC", "BIC", "varchar", 12, 0, false, false, "BIC number of the bank that manages this account (not necessarily required for transactions inside europe)")
      .AddAttribute(6, "BankOwner", "BankOwner", "varchar", 27, 0, false, false, "optionally the name of the owner of the bank details, if this differs from the assigned person")
      .AddAttribute(7, "Country", "Country", "integer", 0, 0, false, false, "ID of the country for the banking entity, key attribute from the \"Countries\" entity")
      
      .AddReference("refBanking2Person", EMyReferenceType::komposition, "Person", "part of relationship from a banking account to the person who own it", { {1,1} })
      .AddReference("refBanking2Type", EMyReferenceType::range, "BankingTypes", "range value to extent the relationship of a person to an banking account", { {2,1} })
      .AddReference("refBanking2Countries", EMyReferenceType::assoziation, "Countries", "key ID to the associated entity of Countries, where bank who manage this account is located", { {7,1} })

      .AddIndex("idx_Banking_IBAN", EMyIndexType::undefined, "access path to search a specific IBAN account number in data", { { 4, true } });

      ;

   dictionary.AddTable("BankingTypes", EMyEntityType::table, "BankingTypes", "dbo", "BankingTypes", "myTest", "System", "SQL", "domain / range of values for banking types, this is an extension of the relationship between persons and banking accounts.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "unique identification number for a record in this domain with banking types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "unique description of an banking type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "non-representative abbreviation that can be used for a short advertisement for this banking type")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "detailed description of these banking type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "notes, with additional / free information for this type of banking, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "boolean value that makes this entity of banking type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_BankingTypes_Denotation", EMyIndexType::key, "unique / representative denotation for an banking type as key canditate for this entity", { { 2, true } });

      ;

   dictionary.AddTable("Countries", EMyEntityType::table, "Countries", "dbo", "Countries", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "CountryDialing", "CountryDialing", "varchar", 10, 0, false, false, "")
      .AddAttribute(6, "ISO_Code", "ISO_Code", "varchar", 5, 0, true, false, "")
      .AddAttribute(7, "Notes", "Notes", "text", 0, 0, false, false, "")
      .AddAttribute(8, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")

      .AddIndex("uk_Countries_Denotation", EMyIndexType::key, "unique / representative denotation for this country entity as key canditate", { { 2, true } })
      .AddIndex("uk_Countries_ISO_Code", EMyIndexType::key, "unique / representative ISO code for this country entity as key canditate", { { 6, true } });

      ;

   dictionary.AddTable("Departments", EMyEntityType::table, "Departments", "dbo", "Departments", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "")

      ;

   dictionary.AddTable("Employees", EMyEntityType::table, "Employees", "dbo", "Employees", "myTest", "System", "SQL", "information about the employees in the company (generalization of a person)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "attribute as foreign key from an attribute ID of a person entity to which these account details belong")
      .AddAttribute(2, "PersonNumber", "PersonNumber", "varchar", 15, 0, true, false, "unique HR number of the employee in the company, assigned by the HR department")
      .AddAttribute(3, "Salery", "Salery", "decimal", 9, 2, false, false, "salary / income that the employee currently receives")
      .AddAttribute(4, "StartOfJob", "StartOfJob", "date", 0, 0, false, false, "starting date of the employee in the company (can be extended later for the start of the current position)")
      .AddAttribute(5, "JobPosition", "JobPosition", "integer", 0, 0, false, false, "current position / activity of the employee within the company")
      .AddAttribute(6, "JobSpec", "JobSpec", "varchar", 100, 0, false, false, "Spezifikation der Tätigkeit als Freitext um dieses zu konkretisieren")
      .AddAttribute(7, "Department", "Department", "integer", 0, 0, true, false, "id of the department where the employee currently work")
      .AddAttribute(8, "SocialNummer", "SocialNummer", "varchar", 20, 0, true, false, "social insurance number of the employee")
      .AddAttribute(9, "Active", "Active", "bool", 0, 0, false, false, "boolean value indicating whether the person is active in the company")

      .AddReference("refEmployees2Person", EMyReferenceType::generalization, "Person", "generalization from an employee to a person (is-a relationship)", { {1,1} })
      .AddReference("refEmployees2JobPositions", EMyReferenceType::range, "JobPositions", "range value as domain for this attribute, possible jobpositions in company", { {5,1} })
      .AddReference("refEmployees2Department", EMyReferenceType::assoziation, "Department", "assoziations between an employee to the department where she/he work", { {7,1} })

      .AddIndex("uk_Employees_PersonNumber", EMyIndexType::key, "unique personal number of a employee in company (key canditate)", { { 2, true } })
      .AddIndex("uk_Employees_SocialNumber", EMyIndexType::key, "extern unique number of an employee (key canditate)", { { 8, true } })
      .AddIndex("idx_Employees_JobSpec", EMyIndexType::undefined, "access path to search for informations about job specifications", { { 6, true } })
      .AddIndex("idx_Employees_Active", EMyIndexType::undefined, "access path to search / filter all active employees", { { 9, true } })

      ;

   dictionary.AddTable("FamilyTypes", EMyEntityType::table, "FamilyTypes", "dbo", "FamilyTypes", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")

      ;

   dictionary.AddTable("FormOfAddress", EMyEntityType::table, "FormOfAddress", "dbo", "FormOfAddress", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "TypeSpec", "TypeSpec", "integer", 0, 0, false, false, "")
      .AddAttribute(6, "Salutation", "Salutation", "varchar", 50, 0, false, false, "")
      .AddAttribute(7, "Valediction", "Valediction", "varchar", 50, 0, false, false, "")
      .AddAttribute(8, "Notes", "Notes", "text", 0, 0, false, false, "")
      .AddAttribute(9, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")

      ;

   dictionary.AddTable("FamilyStatus", EMyEntityType::table, "FamilyStatus", "dbo", "FamilyStatus", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "Coupled", "Coupled", "bool", 0, 0, false, false, "")
      .AddAttribute(6, "NeedDate", "NeedDate", "bool", 0, 0, false, false, "")
      .AddAttribute(7, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")

      ;

   dictionary.AddTable("Internet", EMyEntityType::table, "Internet", "dbo", "Internet", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "InternetType", "InternetType", "integer", 0, 0, true, true, "")
      .AddAttribute(3, "Adresse", "Adresse", "varchar", 100, 0, true, false, "")

      ;

   dictionary.AddTable("InternetTypes", EMyEntityType::table, "InternetTypes", "dbo", "InternetTypes", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "Prefix", "Prefix", "varchar", 10, 0, false, false, "")
      .AddAttribute(6, "Notes", "Notes", "text", 0, 0, false, false, "")
      .AddAttribute(7, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")

      ;
   
   dictionary.AddTable("JobPositions", EMyEntityType::table, "JobPositions", "dbo", "JobPositions", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")
         
      ;

   dictionary.AddTable("Person", EMyEntityType::table, "Person", "dbo", "Person", "myTest", "System", "SQL", "")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
      .AddAttribute(2, "Name", "Name", "varchar", 30, 0, true, false, "")
      .AddAttribute(3, "Firstname", "Firstname", "varchar", 30, 0, false, false, "")
      .AddAttribute(4, "FormOfAddress", "FormOfAddress", "integer", 0, 0, true, false, "")
      .AddAttribute(5, "FamilyStatus", "FamilyStatus", "integer", 0, 0, true, false, "")
      .AddAttribute(6, "FamilyStatusSince", "FamilyStatusSince", "date", 0, 0, false, false, "")
      .AddAttribute(7, "Birthday", "Birthday", "date", 0, 0, false, false, "")
      .AddAttribute(8, "Notes", "Notes", "text", 0, 0, false, false, "")

      ;

    dictionary.AddTable("Phone", EMyEntityType::table, "Phone", "dbo", "Phone", "myTest", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
       .AddAttribute(2, "PhoneType", "PhoneType", "integer", 0, 0, true, true, "")
       .AddAttribute(3, "AreaCode", "AreaCode", "varchar", 10, 0, true, false, "")
       .AddAttribute(4, "CallNumber", "CallNumber", "varchar", 13, 0, true, false, "")
       .AddAttribute(5, "Country", "Country", "integer", 0, 0, true, false, "")

       ;

    dictionary.AddTable("PhonesTypes", EMyEntityType::table, "PhoneTypes", "dbo", "PhonesTypes", "myTest", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
       .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "")
       .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "")
       .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "")
       .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "")
       .AddAttribute(6, "UrgentValue", "UrgentValue", "smallint", 0, 0, false, false, "")

       ;

    dictionary.AddTable("WorkingTime", EMyEntityType::table, "WorkingTime", "dbo", "WorkingTime", "myTest", "System", "SQL", "")
       .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "")
       .AddAttribute(2, "StartingTime", "StartingTime", "datetime", 0, 0, true, true, "")
       .AddAttribute(3, "ClosingTime", "ClosingTime", "datetime", 0, 0, false, false, "")
       .AddAttribute(4, "Processed ", "Processed ", "bool", 0, 0, true, false, "")

       ;
   }
