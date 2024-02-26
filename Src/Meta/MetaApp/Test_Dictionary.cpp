#include "Test_Dictionary.h"

#include <sstream>

TDictionary_Test::TDictionary_Test() : dictionary("simple person model") {
   std::ostringstream os1, os2;

   dictionary.BaseClass("TSimplePersonBase");
   dictionary.BaseNamespace("myCorporate");
   dictionary.PathToBase("System/Corporate/base.h");


   os1 << "Model for a simple management of personal data for testing and presenting the use of "
       << "metadata with a generator and for discussing the possibilities of using metadata.\n"
       << "The model is implemented using the adecc Scholar metadata class TMyDictionary and "
       << "the associated classes TMyDatatype, TMyAttribute, TMyTable, TMyReferences and TMyIndices.\n"
      
       << "In addition to the necessary C++ source code for the classes of the system layer, this generator "
       << "also creates scripts for creating the database with conditions and value tables and the basis for "
       << "detailed documentation with Doxygen in separate files. In addition, methods are created that enable "
       << "access using the database interface of the adeccDatabase library developed in this stream.\n"

       << "The advantage of using generators that work on the basis of metadata is not only the enormous time saving, "
       << "but also the correctness. This means that all parts are up-to-date and 100% aligned with each other.\n"

       << "The basis for this model was taken from a training project by adecc Systemhaus GmbH and expanded once again. "
       << "The following image show a draft paper / scratch pad of the background for this trainings project."
       << "\\image html Kladde.jpg\n"
       << "\\image latex Kladde.jpg\n"

       << "The following model diagram is created with the Embarcader E/R Studio and show the initial model from the former project."

       << "\\image html ER-Model.jpg\n"
       << "\\image latex ERModelLS.jpg";
 
   os1 << "This model was created to show and discuss the potential of metadata and the generation "
       << "of source code, database scripts and documentation.\n"
       << "In the model, in addition to the is - a relationship between the Person and Employee tables, "
       << "there is a part - of relationship to employee working hours and an association to the departments "
       << "in which employees work.\n"
       << "In addition, there are a number of extended value tables to show that these do not only have to "
       << "consist of an ID and name, but can also contain other data.\n"
       << "The project, including all additional files and documentations, are part of the source code of the "
       << "\"adecc Scholar\" project. The purpose of this project is to impart know-how on architecture and "
       << "technology when using the programming language C++. The MIT license applies unless otherwise stated "
       << "or other licenses must be used due to legal requirements.";

   os2 << "There are many discussions about programming guidelines, names of identifiers, the pros and cons of "
       << "notations (Hungarian notation, all lowercase, first letter uppercase, ...). These discussions also "
       << "fade against the background of programming with metadata and can easily be adapted to any standard "
       << "without any significant effort.\n" 
       << "Finally, please note that these sources are only training materials and explicitly not productive "
       << "sources. You can use them according to the license in your own application, but there is no claim "
       << "to correctness or any form of warranty.";   

   

   /*
   * 
   * doxygen - w latex headerFile footerFile styleSheetFile
   * 
   * example for a landscape document
   *    \documentclass{article}
   *    \usepackage[a4paper,margin=1in,landscape]{geometry}
   *    \usepackage{blindtext}
   *    \usepackage{tikz}
   *    \usetikzlibrary{calc}
   *    \begin{document}
   *    \begin{tikzpicture}[remember picture,overlay]
   *       \node at ($(current page.north east) + (-1cm,-1cm)$) {NE};
   *    \end{tikzpicture}
   *    \blinddocument
   *    \end{document}
   */


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

   dictionary.AddNameSpace("myCorporate", "namespace with the classes that are used jointly by all parts of the company",
                           "This area contains all classes, enumerations and data structures that are shared by different parts of the "
                           "company. These types are the central services in all programs that are developed in the corporate environment "
                           "and are further developed by a central department that is controlled by a board made up of those responsible for " 
                           "the other areas.",
                           "This area is under responsibility of the central services department.");

   dictionary.AddNameSpace("myHR", "namespace with the classes that are developed for use in the department of human resources.",
                           "This area contains the classes and data structures that are used in the personnel administration area. "
                           "In addition to the classes for managing employees and departments, there are also payroll accounting and "
                           "data for these.\n"
                           "In time accounting, there is an interface to the development and sales department, as resources can be used "
                           "here and links exist in the tables.\n"
                           "All areas are based on the namespace myCorporate .",
                           "This area is under the responsibility of the HR department");

   dictionary.AddDirectory(".", "root folder for the source of the project.",
                           "This folder contain all parts of the applications and libs for the project.");
   dictionary.AddDirectory("System", "subfolder with header and source files of the system layer for the project.",
                           "This folder contains all header and source files for the system layer of the application.\n"
                           "Its possible that you will find Subfolder for different namespaces here.");

   dictionary.AddDirectory("System/Corporate", "subfolder with header and source files for the common source files.",
                           "This folder contains all header and source files for the corparate system layer.");

   dictionary.AddDirectory("System/HR", "subfolder with header and source files for the source files for human resources.",
                            "This folder contains all header and source files for the system layer of the part of "
                            "human resources");

   dictionary.AddTable("Address", EMyEntityType::table, "Address", "dbo", "Address", "myCorporate", "System/Corporate", "SQL", "information on the addresses where a person lives, works or has any other relationship with them")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "attribute as Foreign key from an attribute ID of the person entity to whom the address belongs")
      .AddAttribute(2, "AddressType", "AddressType", "integer", 0, 0, true, true, "", "", "", "extension of the key by the address type to manage different addresses for one person")
      .AddAttribute(3, "Zipcode", "Zipcode", "varchar", 10, 0, true, false, "", "", "", "Zip code for the address")
      .AddAttribute(4, "City", "City", "varchar", 50, 0, true, false, "", "", "", "name of the city/county (possibly with district) for this address")
      .AddAttribute(5, "Street", "Street", "varchar", 50, 0, true, false, "", "", "", "name of the street belonging to this address")
      .AddAttribute(6, "StreetNumber", "StreetNumber", "varchar", 10, 0, false, false, "", "", "", "house number with addition for this address")
      .AddAttribute(7, "Country", "Country", "integer", 0, 0, true, false, "", "", "", "ID of the country for the address, key attribute from the Countries entity")

      .AddReference("refAddress2Person", EMyReferenceType::composition, "Person", "residential address", "1 : n", { }, "part of- relationship from a person to the residential addresses", { {1,1} })
      .AddReference("refAddress2AddressType", EMyReferenceType::range, "AddressTypes", "has values", "n : 1", { 2 }, "range value to extent the relationship of a person to an address", { {2,1} })
      .AddReference("refAddress2Countries", EMyReferenceType::assoziation, "Countries", "belongs to", "n : 1", { 2 }, "key ID to the associated entity of Countries, where this address is located", { {7,1} })

      .AddIndex("idx_Address_City_Street", EMyIndexType::undefined, "access path to search for city / street combinations in addresses", { { 4, true }, { 5, true } });
      ;

   dictionary.AddTable("AddressTypes", EMyEntityType::range, "AddressTypes", "dbo", "AddressTypes", "myCorporate", "System/Corporate", "SQL", "domain / range of values for address types, this is an extension of the relationship between persons and addresses.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with address types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of an entry in the domain, display in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this address")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these address types, what is associated with them. Self documentation, possible use as hint")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this AddressType, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of address type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_AddressType_Denotation", EMyIndexType::key, "unique / representative denotation for an addresstye as key canditate", { { 3, true } })
 
      .AddRangeValue("INSERT INTO AddressTypes (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
                     "    (1, 'Hauptadresse', 'HA', 'Hauptadresse für die Anwendung', 1), \n"
                     "    (2, 'Rechnungsadresse', 'RA', 'Rechnungsadresse für die Anwendung', 0), \n"
                     "    (3, 'Lieferadresse', 'LA', 'Lieferadresse für die Trainingsanwendung', 0), \n"
                     "    (4, 'Urlaubsadresse', 'UA', 'eine Adresse, in der die Person im Urlaub erreichbar ist', 0)")

      ;

   dictionary.AddTable("Banking", EMyEntityType::table, "Banking", "dbo", "Banking", "myCorporate", "System/Corporate", "SQL", "informations about the account details provided by a person and used in a specific context")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "attribute as foreign key from an attribute ID of a person entity to which these account details belong")
      .AddAttribute(2, "BankingType", "BankingType", "integer", 0, 0, true, true, "", "", "", "extension of the account details key in order to be able to save several / different bank details in a standardized way")
      .AddAttribute(3, "BankName", "BankName", "varchar", 50, 0, false, false, "", "", "", "name of the bank that manages this account (not necessarily required)")
      .AddAttribute(4, "IBAN", "IBAN", "varchar", 33, 0, true, false, "", "", "", "IBAN number (account identification) for this bank account (required)")
      .AddAttribute(5, "BIC", "BIC", "varchar", 12, 0, false, false, "", "", "", "BIC number of the bank that manages this account (not necessarily required for transactions inside europe)")
      .AddAttribute(6, "BankOwner", "BankOwner", "varchar", 27, 0, false, false, "", "", "", "optionally the name of the owner of the bank details, if this differs from the assigned person")
      .AddAttribute(7, "Country", "Country", "integer", 0, 0, false, false, "", "", "", "ID of the country for the banking entity, key attribute from the Countries entity")

      .AddReference("refBanking2Person", EMyReferenceType::composition, "Person", "owns", "1 : n", { }, "part of relationship from a banking account to the person who own it", { {1,1} })
      .AddReference("refBanking2Type", EMyReferenceType::range, "BankingTypes", "has values", "n : 1", { 2 }, "range value to extent the relationship of a person to an banking account", { {2,1} })
      .AddReference("refBanking2Countries", EMyReferenceType::assoziation, "Countries", "belongs to", "n : 1", { 2 }, "key ID to the associated entity of Countries, where bank who manage this account is located", { {7,1} })

      .AddIndex("idx_Banking_IBAN", EMyIndexType::undefined, "access path to search a specific IBAN account number in data", { { 4, true } })
      .AddIndex("idx_Banking_BIC", EMyIndexType::undefined, "access path to search a specific BIC in data", { { 5, true } })

      ;

   dictionary.AddTable("BankingTypes", EMyEntityType::range, "BankingTypes", "dbo", "BankingTypes", "myCorporate", "System/Corporate", "SQL", "domain / range of values for banking types, this is an extension of the relationship between persons and banking accounts.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with banking types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of an banking type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this banking type")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these banking type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of banking, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of banking type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_BankingTypes_Denotation", EMyIndexType::key, "unique / representative denotation for an banking type as key canditate for this entity", { { 2, true } })

      .AddRangeValue("INSERT INTO BankingTypes (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
                     "    (1, 'Hauptkonto', 'HK', 'Hauptkonto für die Personen in der Anwendung', 1)")

      ;

   dictionary.AddTable("Countries", EMyEntityType::table, "Countries", "dbo", "Countries", "myCorporate", "System/Corporate", "SQL", "table with the countries and additional values for the countries. Used by others for assoziation.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in the table Countries")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique, official designation of the country, key candidate")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "abbreviation of the country for the program (iso code 3166 - alpha 3)")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "text field for the description of the country. can be used by the program to provide additional information, for example in the status bar or a hint")
      .AddAttribute(5, "CountryDialing", "CountryDialing", "varchar", 10, 0, false, false, "", "", "", "international country code for telephone for this country (not unique)")
      .AddAttribute(6, "ISO_Code", "ISO_Code", "varchar", 5, 0, false, false, "", "", "", "unique, official international code (ISO code 3166 - alpha 2) for the country in the communication")
      .AddAttribute(7, "IsEU", "IsEU", "bool", 0, 0, false, false, "", "", "", "boolean value which is true when the country part of the european union")
      .AddAttribute(8, "Capital", "Capital", "varchar", 50, 0, false, false, "", "", "", "name of the capital of this country ")
      .AddAttribute(9, "Currency", "Currency", "varchar", 50, 0, false, false, "", "", "", "currency in this country")
      .AddAttribute(10, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this country, not used in application")
      .AddAttribute(11, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of country to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_Countries_Denotation", EMyIndexType::key, "unique / representative official name of the country as key canditate for this entity", { { 2, true } })
      .AddIndex("uk_Countries_ISOCode", EMyIndexType::key, "unique / representative official iso code of the country as key canditate for this entity", { { 6, true } })

      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function
      .AddPostConditions("CREATE FUNCTION [dbo].[Country_Dialing] (@id AS INTEGER) RETURNS VARCHAR(10)\n"
                         "BEGIN\n"
                         "   DECLARE @retval VARCHAR(10);\n"
                         "   SELECT @retval = CountryDialing\n"
                         "   FROM dbo.Countries\n"
                         "   WHERE ID = @id;\n"
                         "   RETURN @retval;\n"
                         "END")
      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function

      .AddCleanings("DROP FUNCTION [dbo].[Country_Dialing]")
      ;

   dictionary.AddTable("Departments", EMyEntityType::table, "Departments", "dbo", "Departments", "myHR", "System/HR", "SQL", "independent entity with the departments in the company, independent identity")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identifications number of the department. primary key and used for assoziations")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "distinct name of the department in the company. used to identify this and use in application for select")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "used abbreviation of the department for list and such")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "description for this department, is used in the program, for example as hint ")
      .AddAttribute(5, "Officer", "Officer", "integer", 0, 0, false, false, "", "", "", "person responsible / head of this department (association with the employys entity)")
      .AddAttribute(6, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "free notes to record information that is not used in the program")

      .AddReference("refDepartments2Employee", EMyReferenceType::assoziation, "Employees", "responsible", "n : 1", { 9 }, "assoziation between the department to employee who is the responsible officer for this", { {5,1} })

      .AddIndex("uk_Departments_Denotation", EMyIndexType::key, "unique / representative denotation forthe department as key canditate for this entity", { { 2, true } })
      .AddIndex("idx_Departments_Abbr", EMyIndexType::undefined, "search pass for abbreviations of departments", { { 3, true } })

      ;

   dictionary.AddTable("Employees", EMyEntityType::table, "Employees", "dbo", "Employees", "myHR", "System/HR", "SQL", "information about the employees in the company (generalization of a person)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "attribute as foreign key from an attribute ID of a person entity to which these account details belong")
      .AddAttribute(2, "PersonNumber", "PersonNumber", "varchar", 15, 0, true, false, "", "", "", "unique HR number of the employee in the company, assigned by the HR department")
      .AddAttribute(3, "Salery", "Salery", "decimal", 10, 2, false, false, ">= 0.0", "", "", "salary / income that the employee currently receives, dependant by the SalaryBase")
      .AddAttribute(4, "SalaryType", "SalaryType", "integer", 0, 0, false, false, "", "", "", "range for SalaryType with the kind of Salary and determine the SalaryBase for the calculation")
      .AddAttribute(5, "TaxClass", "TaxClass", "integer", 0, 0, true, false, "", "", "", "Tax class currently held by the employee")
      .AddAttribute(6, "StartOfJob", "StartOfJob", "date", 0, 0, true, false, "", "", "", "starting date of the employee in the company (can be extended later for the start of the current position)")
      .AddAttribute(7, "EndOfJob", "EndOfJob", "date", 0, 0, false, false, "[EndOfJob IS NULL OR (EndOfJob > StartOfJob)]", "", "", "end date of the employee in the company, all dates without time history")
      .AddAttribute(8, "ReasonDeparture", "ReasonDeparture", "integer", 0, 0, false, false, "[(ReasonDeparture IS NULL AND EndOfJob IS NULL) OR  (ReasonDeparture IS NOT NULL AND EndOfJob IS NOT NULL)]", "", "", "reason of departture when the person separated and the emplyment finished (NULL if active)")
      .AddAttribute(9, "JobPosition", "JobPosition", "integer", 0, 0, false, false, "", "", "", "current position / activity of the employee within the company")
      .AddAttribute(10, "JobSpec", "JobSpec", "varchar", 100, 0, false, false, "", "", "", "specification of the job as free text to concretize this")
      .AddAttribute(11, "VacationDays", "VacationDays", "unsigned", 0, 0, false, false, "", "", "", "Entitlement to annual vacations for this employee")
      .AddAttribute(12, "Department", "Department", "integer", 0, 0, true, false, "", "", "", "id of the department where the employee currently work")
      .AddAttribute(13, "SocialNummer", "SocialNummer", "varchar", 20, 0, true, false, "", "", "", "social insurance number of the employee")
      .AddAttribute(14, "Active", "Active", "bool", 0, 0, false, false, "", "", "[IIF(EndOfJob IS NULL OR EndOfJob >= GETDATE(), 1, 0)]", "calculated boolean value indicating whether the person is active in the company")

      .AddReference("refEmployees2Person", EMyReferenceType::generalization, "Person", "is-a", "1 : 1", { 9 }, "generalization from an employee to a person (is-a relationship)", { {1,1} })
      .AddReference("refEmployees2SalaryType", EMyReferenceType::range, "SalaryType", "has values", "n : 1", { 2 }, "range values as domain for the attribute SalaryBase", { {4,1} })
      .AddReference("refEmployees2TaxClass", EMyReferenceType::range, "TaxClasses", "has values", "n : 1", { 2 }, "range value as domain for the attribute TaxClass", { {5,1} })
      .AddReference("refEmployees2ReasonDeparture", EMyReferenceType::range, "ReasonDeparture", "separation because", "n : 1", { 2 }, "range value as domain for the reason for departure of the employee", { {8,1} })
      .AddReference("refEmployees2JobPositions", EMyReferenceType::range, "JobPositions", "holds", "n : 1", { 2 }, "range value as domain for this attribute, possible jobpositions in company", { {9,1} })
      .AddReference("refEmployees2Department", EMyReferenceType::assoziation, "Departments", "works in", "n : 1", { 2 }, "assoziations between an employee to the department where she/he work", { {12,1} })

      .AddIndex("uk_Employees_PersonNumber", EMyIndexType::key, "unique personal number of a employee in company (key canditate)", { { 2, true } })
      .AddIndex("uk_Employees_SocialNumber", EMyIndexType::key, "extern unique number of an employee (key canditate)", { { 8, true } })
      .AddIndex("idx_Employees_JobSpec", EMyIndexType::undefined, "access path to search for informations about job specifications", { { 6, true } })
      .AddIndex("idx_Employees_Active", EMyIndexType::undefined, "access path to search / filter all active employees", { { 9, true } })

      ;

   dictionary.AddTable("FamilyStatus", EMyEntityType::range, "FamilyStatus", "dbo", "FamilyStatus", "myCorporate", "System/Corporate", "SQL", "domain / range of values for family status, this is used in person to qualifify the actual status, maybe a date needed too")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with family status")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of a family status entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this family status")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these family status,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Coupled", "Coupled", "bool", 0, 0, false, false, "", "", "", "boolean value that show if persones with this status are coupled and possibly receive tax relief")
      .AddAttribute(6, "NeedDate", "NeedDate", "bool", 0, 0, false, false, "", "", "", "boolean value which determine that the date when the person status changed the last time needed. ")
      .AddAttribute(7, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of family status, not used in application")
      .AddAttribute(8, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of phone type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_FamilyStatus_Denotation", EMyIndexType::key, "unique / representative denotation for a family status as key canditate for this entity", { { 2, true } })

      .AddRangeValue("INSERT INTO FamilyStatus (ID, Denotation, Abbreviation, Description, Coupled, NeedDate, UrgentValue) VALUES\n"
                     "    (0, 'unbekannt', 'kA', 'unbekannt oder keine Angabe', 0, 0, 1), \n"
                     "    (1, 'ledig', 'ldg', 'ledige Person', 0, 0, 1), \n"
                     "    (2, 'verheiratet', 'verh', 'verheiratete Person', 1, 1, 1), \n"
                     "    (3, 'geschieden', 'gesch', 'geschiedene Person', 0, 1, 1), \n"
                     "    (4, 'verwitwert', 'verw', 'verwitwerte Person', 0, 1, 1), \n"
                     "    (5, 'Lebenspartnerschaft', 'LP', 'eingetragene Lebenspartnerschaft', 1, 1, 0)")

      ;

   dictionary.AddTable("FamilyTypes", EMyEntityType::range, "FamilyTypes", "dbo", "FamilyTypes", "myCorporate", "System/Corporate", "SQL", "domain / range of values for family types, this is used in form of address to qualifify the person gender")
      .AddAttribute(  1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with family types")
      .AddAttribute(  2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of a family type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(  3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this banking type")
      .AddAttribute(  4, "IsNaturalPerson", "IsNaturalPerson", "bool", 0, 0, false, false, "", "", "", "boolean indicator whether this entity is a natural person when true, otherwise ist a legal entity")
      .AddAttribute(  5, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these family type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(  6, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of family status, not used in application")
      .AddAttribute(  7, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of family type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_FamilyTypes_Denotation", EMyIndexType::key, "unique / representative denotation for a family type (choosen gender) as key canditate for this entity", { { 2, true } })

      .AddRangeValue("INSERT INTO FamilyTypes (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
                     "    (1, 'Mann', 'M', 'Person mit männlichem Geschlecht', 1), \n"
                     "    (2, 'Frau', 'F', 'Person mit weiblichen Geschlecht', 1), \n"
                     "    (3, 'Diverse', 'div', 'Eine Identität, die die Vielfalt der Geschlechteridentitäten umfasst und nicht in die traditionellen binären Kategorien männlich oder weiblich passt.', 1), \n"
                     "    (4, 'Non-Binär', 'non', 'Eine Identität außerhalb der traditionellen binären Geschlechtkategorien männlich und weiblich, die sich jenseits dieser Konzepte befindet.', 1), \n"
                     "    (5, 'Firma', 'FA', 'Eine rechtliche Einheit, die gegründet wird, um kommerzielle oder geschäftliche Aktivitäten auszuüben, die in der Regel einen oder mehrere Eigentümer haben.', 1), \n"
                     "    (6, 'Familie', 'Fam', 'Eine soziale Einheit, die aus Eltern und ihren Kindern besteht, die zusammenleben und sich normalerweise gegenseitig unterstützen und emotional miteinander verbunden sind.', 1), \n"
                     "    (7, 'Gemeinschaft', 'eLG', 'Eine Form des Zusammenlebens von Menschen, die nicht notwendigerweise auf einer romantischen Beziehung basiert, sondern auf gemeinsamen Zielen, Werten oder Lebensweisen beruht, oft in einem gemeinsamen Haushalt.', 1), \n"
                     "    (8, 'Verein', 'EV', 'Eine rechtliche und organisatorische Struktur, die von einer Gruppe von Personen gegründet wurde, um gemeinsame Interessen oder Ziele zu verfolgen, oft auf gemeinnütziger Basis.', 1)")

      ;

   dictionary.AddTable("FormOfAddress", EMyEntityType::range, "FormOfAddress", "dbo", "FormOfAddress", "myCorporate", "System/Corporate", "SQL", "domain with the possible values for form of addresses with additional informations for this")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number of the form of address to use it in relationships")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of  this form of address entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this form of address")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these form of address,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "TypeSpec", "TypeSpec", "integer", 0, 0, false, false, "", "", "", "value from FamilyTypes who determine the art of persontype (gender, legal form, …)")
      .AddAttribute(6, "Salutation", "Salutation", "varchar", 50, 0, false, false, "", "", "", "salutation formula to be used in letters and emails for this salutation type and therefore person type")
      .AddAttribute(7, "Valediction", "Valediction", "varchar", 50, 0, false, false, "", "", "", "farewell formula to be used in letters and emails for this salutation type and therefore person type")
      .AddAttribute(8, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of form of address, not used in application")
      .AddAttribute(9, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of form of address type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_FormOfAddress_Denotation", EMyIndexType::key, "unique / representative denotation for a form of address", { { 2, true } })

      .AddReference("refFormOfAddress2Type", EMyReferenceType::assoziation, "FamilyTypes", "represent", "n : 1", { 2 }, "assoziation between a form of address and a familiy type which is represent", { {5,1} })
 
      .AddRangeValue("INSERT INTO FormOfAddress (ID, Denotation, Abbreviation, Description, TypeSpec, Salutation, Valediction, UrgentValue) VALUES\n"
                     "    (1, 'Herr', 'Hr', 'Anredesteuerung für Herr', 1, 'Sehr geehrter Herr', 'Mit freundlichen Grüßen', 1), \n"
                     "    (2, 'Frau', 'Fr', 'Anredesteuerung für Frauen', 2, 'Sehr geehrte Frau', 'Mit freundlichen Grüßen', 1), \n"
                     "    (3, 'Firma', 'Fa', 'Anredesteuerung für Firmen', 3, 'Sehr geehrte Damen und Herren', 'Mit freundlichen Grüßen', 0)")
     
      ;

   dictionary.AddTable("Internet", EMyEntityType::table, "Internet", "dbo", "Internet", "myCorporate", "System/Corporate", "SQL", "connections for different kinds of communications about the internet as part of persons (part of relationship)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number of the person to whom the internet connection data belongs (foreign key from person)")
      .AddAttribute(2, "InternetType", "InternetType", "integer", 0, 0, true, true, "", "", "", "type of internet connection data (value range from InternetType as foreign key, extension to 1:n relationship))")
      .AddAttribute(3, "Adresse", "Adresse", "varchar", 100, 0, true, false, "", "", "", "internet address for this connection, possible to split the protocol to the value table InternetTypes")

      .AddReference("refInternet2Person", EMyReferenceType::composition, "Person", "owns", "1 : n", { }, "part of relationship from an internet connection to the person who own it", { {1,1} })
      .AddReference("refInternet2Type", EMyReferenceType::range, "InternetTypes", "has values", "n : 1", { 2 }, "range value to extent the relationship of a person to an internet connection", { {2,1} })

      .AddIndex("idxInternet_Address", EMyIndexType::undefined, "access path to seach a address of internet connection", { { 3, true } })
      ;

   dictionary.AddTable("InternetTypes", EMyEntityType::range, "InternetTypes", "dbo", "InternetTypes", "myCorporate", "System/Corporate", "SQL", "domain / range of values for internet connection types, this is an extension of the relationship between persons and internet connections.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with internet connection types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of an internet connection type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this internet connection type")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these internet connection type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Prefix", "Prefix", "varchar", 10, 0, false, false, "", "", "", "")
      .AddAttribute(6, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of internet connection, not used in application")
      .AddAttribute(7, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of internet connection type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_InternetTypes_Denotation", EMyIndexType::key, "unique / representative denotation for this internet type", { { 2, true } })

      .AddRangeValue("INSERT INTO InternetTypes (ID, Denotation, Abbreviation, Description, Prefix, UrgentValue) VALUES\n"
                     "    (1, 'Internet', 'HTTP', 'Die Homepage einer Person', 'http://', 1), \n"
                     "    (2, 'Email', 'Mail', 'Email- Adresse einer Person', 'mailto:', 1), \n"
                     "    (3, 'Skype', 'Skype', 'Skype- Name einer Person', 'skype:', 1), \n"
                     "    (4, 'Telefon', 'VoIP', 'VoIP- Nummer einer Person', 'phone:', 1)")

      ;
   
   dictionary.AddTable("JobPositions", EMyEntityType::table, "JobPositions", "dbo", "JobPositions", "myHR", "System/HR", "SQL", "domain or range of values for different positions for the employee. Important as anchor for additional informations")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with positions in job")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique denotation for this job position, used in selections, and lists to descripe this position")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "abbreviation for this job position, used in overvies. Can be null and don't need be unique")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these job position, this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "SalaryType", "SalaryType", "integer", 0, 0, false, false, "", "", "", "prefered salary type for this job positions. may be overridden in emplyee for individual aggreements")
      .AddAttribute(6, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of job position, not used in application")
      .AddAttribute(7, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of job position to a system value (cannot be changed as it is used directly by the program)")

      .AddReference("refJobPosition2SalaryType", EMyReferenceType::range, "SalaryType", "is in this", "n : 1", { 2 }, "range value which descripe which kind of salary this JobPositions use", { {1,1} })

      .AddIndex("uk_JobPositions_Denotation", EMyIndexType::key, "unique / representative denotation for this position in job", { { 2, true } })

      ;

   dictionary.AddTable("Person", EMyEntityType::table, "Person", "dbo", "Person", "myCorporate", "System/Corporate", "SQL", "informations about a person, base for different kinds of special persons in other areas of the company")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a entity of person")
      .AddAttribute(2, "Name", "Name", "varchar", 30, 0, true, false, "", "", "", "family name of the natural person respectively the name for a legal entity")
      .AddAttribute(3, "Firstname", "Firstname", "varchar", 30, 0, false, false, "", "", "", "first name of the natural person respectively a name extenstion for a legal entity")
      .AddAttribute(4, "FormOfAddress", "FormOfAddress", "integer", 0, 0, true, false, "", "", "", "form of address for this person. this attribute control the kind of a person too")
      .AddAttribute(5, "FamilyStatus", "FamilyStatus", "integer", 0, 0, true, false, "", "", "", "family status for the person")
      .AddAttribute(6, "FamilyStatusSince", "FamilyStatusSince", "date", 0, 0, false, false, "", "", "", "the date from which the current family status applies.")
      .AddAttribute(7, "Birthday", "Birthday", "date", 0, 0, false, false, "", "", "", "birthday of a natural person. This attribute is unused for legal entrities")
      .AddAttribute(8, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this tperson, not used in application")
      .AddAttribute(9, "FullName", "FullName", "varchar", 60, 0, false, false, "", "", "Name + ', ' + FirstName", "calculated field for displaying the full name for use in the program, for example, if a person is to be selected")

      .AddReference("refPerson2FormOfAddress", EMyReferenceType::range, "FormOfAddress", "has values", "n : 1", { 2 }, "range value to extent the relationship of a person to a form of address", { {4,1} })
      .AddReference("refPerson2FamilyStatus", EMyReferenceType::range, "FamilyStatus", "has values", "n : 1", { 2 }, "range value to extent the relationship of a person to a family status", { {5,1} })

      .AddIndex("idx_Person_Name_Firstname", EMyIndexType::undefined, "access path to search for name and / or firstname in a person", { { 2, true }, { 3, true } })
      ;

   dictionary.AddTable("Phone", EMyEntityType::table, "Phone", "dbo", "Phone", "myCorporate", "System/Corporate", "SQL", "phone connections of persons (part of relationship)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number of the person to whom the phone data belongs (foreign key from person)")
      .AddAttribute(2, "PhoneType", "PhoneType", "integer", 0, 0, true, true, "", "", "", "type of telephone data (value range from PhoneType as foreign key, extension to 1:n relationship))")
      .AddAttribute(3, "AreaCode", "AreaCode", "varchar", 10, 0, true, false, "", "", "", "area code or network code of the telephone connection")
      .AddAttribute(4, "CallNumber", "CallNumber", "varchar", 13, 0, true, false, "NOT LIKE '%[^0-9 -]%'", "", "", "call number within the local network or the selected cellphone network")
      .AddAttribute(5, "Country", "Country", "integer", 0, 0, true, false, "", "", "", "country id for this data as foreign key in the Countries table. the country code is also located here")
      .AddAttribute(6, "DialingNational", "DialingNational", "varchar", 25, 0, false, false, "", "", "AreaCode + REPLACE(REPLACE(CallNumber, ' ', ''), '-', '')", "complete telephone number to be dialed in the national network as computed field")
      .AddAttribute(7, "DialingInternational", "DialingInternational", "varchar", 30, 0, false, false, "", "", "[[dbo].[Country_Dialing](Country)+SUBSTRING(AreaCode, 2, LEN(AreaCode))+CallNumber]", "complete telephone number to be dialed in the international network as computed field")

      .AddReference("refPhone2Person", EMyReferenceType::composition, "Person", "owns", "1 : n", { 2 }, "part of relationship from a phone number to the person who own it", { {1,1} })
      .AddReference("refPhone2Type", EMyReferenceType::range, "PhonesTypes", "has values", "n : 1", { 2 }, "range value to extent the relationship of a person to a phone number", { {2,1} })
      .AddReference("refPhone2Countries", EMyReferenceType::assoziation, "Countries", "belongs to", "n : 1", { }, "key ID to the associated entity of Countries, where phone connection located", { {5,1} })

      .AddIndex("idx_Phone_Number", EMyIndexType::undefined, "access path to search a phone number", { { 3, true }, { 4, true } })
      ;

   dictionary.AddTable("PhonesTypes", EMyEntityType::range, "PhoneTypes", "dbo", "PhonesTypes", "myCorporate", "System/Corporate", "SQL", "domain / range of values for phone types, this is an extension of the relationship between persons and phone.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with phone types")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description of a phone type entity in the domain, used in selections, comboboxes, … (key canditate)")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "non-representative abbreviation that can be used for a short advertisement for this phone  type")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "detailed description of these phone  type,this is a self documentation, possible use as hint or to information in the programm")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for this type of banking, not used in application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of phone type to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_PhonesTypes_Denotation", EMyIndexType::key, "unique / representative denotation for thistype of phone connections", { { 2, true } })

      .AddRangeValue("INSERT INTO PhoneTypes (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
                     "    (1, 'Telefon', 'Tel', 'Telefonnummer zu einer Person', 1), \n"
                     "    (2, 'Fax', 'Fax', 'Faxnummer zu einer Person', 1), \n"
                     "    (3, 'Mobil', 'Mbl', 'Mobiltelefonnummer zu einer Person', 1)")

       ;

   dictionary.AddTable("ReasonDeparture", EMyEntityType::range, "ReasonDeparture", "dbo", "ReasonDeparture", "myHR", "System/HR", "SQL", "doman range with the reason of departure of the employee")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number / id of the reason of the departure")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique denotation for the reason of the departure")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "abbreviation for the reason of the departure, used in the application for a compact display")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "description as long text for the reason of the departure, used in the application for detailed informations")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for the reason of the departure, not used in the application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of reason of departure to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_ReasonDeparture_Denotation", EMyIndexType::key, "unique / representative denotation for this reason of departure", { { 2, true } })

      .AddRangeValue("INSERT INTO ReasonDeparture (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
                     "    (1, 'Kündigung', 'Kü', 'Die Person hat freiwillig beschlossen, das Unternehmen zu verlassen (Resignation).', 1), \n"
                     "    (2, 'Rente', 'Re', 'Die Person verlässt das Unternehmen aufgrund des Erreichens des Rentenalters oder aus Altersgründen (Retirement ).', 1), \n"
                     "    (3, 'Entlassung', 'En', 'Das Unternehmen hat das Arbeitsverhältnis mit der Person beendet (Dismissal )', 1), \n"
                     "    (4, 'Restrukturierung', 'St', 'Die Person wird nicht mehr in diesem Programm geführt, da sie in einem anderen Unternehmensteil beschäftigt wird (restructuring).', 1)")

      ;

   dictionary.AddTable("ReasonNonWorking", EMyEntityType::range, "ReasonNonWorking", "dbo", "ReasonNonWorking", "myHR", "System/HR", "SQL", "domain / range with the reasons of non working time / idle time")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number / id of the reason of non working times / idle times")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique denotation for the reason of the non working times / idle times")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "abbreviation for the reason of the non working times / idle times, used in the application for a compact display")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "description as long text for the reason of the non working times / idle times, used in the application for detailed informations")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "notes, with additional / free information for the reason of the non working times / idle times, not used in the application")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of reason of departure to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_ReasonNonWorking_Denotation", EMyIndexType::key, "unique / representative denotation for this reason of idle time", { { 2, true } })

      .AddRangeValue("INSERT INTO ReasonNonWorking (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
                     "    (1, 'Urlaub', 'U', 'Die Person hat regulären Urlaub (inklusive eventuelle Tage Resturlaub', 1), \n"
                     "    (2, 'Sonderurlaub', 'SU', 'Die Person hat Sonderurlaub', 1), \n"
                     "    (3, 'Krankheit bezahlt', 'K', 'Die Person ist krankgeschrieben', 1), \n"
                     "    (4, 'Krankheit unbezahlt', 'KU', 'Die Person ist krankgeschrieben, aber die Krankenkasse zahlt.', 1), \n"
                     "    (5, 'unbezahlt', 'unbez', 'Die Person ist unbezahlt freigestellt', 1)")

      ;

   dictionary.AddTable("SalaryBase", EMyEntityType::range, "SalaryBase", "dbo", "SalaryBase", "myHR", "System/HR", "SQL", "fix domain for the calculation, this value determine the base for the salary, used in table SalaryType")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "ID for this Base of Salary (0 = free, 1 = hourly, 2 = dayly, 3 = monthly). Extentable via model, needed for calculations")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique description / donation for this base of salary. The algorithm use the id , the text is for display or select")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "unique abbreviation for this base of salary. The algorithm use the id , the text is for display or select")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "a longer description for this base to use as hint or for display")
      .AddAttribute(5, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "")

      .AddIndex("uk_SalaryBase_Denotation", EMyIndexType::key, "unique / representative denotation for this base of salary", { { 2, true } })
      .AddIndex("uk_SalaryBase_Abbreviation", EMyIndexType::key, "unique / representative abbreviation for this base of salary", { { 3, true } })

      .AddRangeValue("INSERT INTO SalaryBase (ID, Denotation, Abbreviation, Description, UrgentValue) VALUES\n"
         "    (0, 'ohne', 'f', 'Die Angabe des Gehalts wird ignoriert oder einmalig gezahlt. Kostenloses Praktikum oder Probezeit.', 1), \n"
         "    (1, 'stündlich', 'h', 'Die Angabe des Gehalts erfolgt auf Stundenbasis. Entspricht meistens dem Arbeitslohn, eventuell mit Zuschlägen.', 1), \n"
         "    (2, 'täglich', 'd', 'Die Angabe des Gehalts erfolgt auf Tagesbasis. Dieses wird oft für Honorare verwendet.', 1), \n"
         "    (3, 'monatlich', 'm', 'Die Angabe des Bezugs erfolgt auf Monatsbasis.Das entspricht meistens dem Gehalt bei Angestellten*innen', 1)")

      ;

   dictionary.AddTable("SalaryType", EMyEntityType::range, "SalaryType", "dbo", "SalaryType", "myHR", "System/HR", "SQL", "kind of salary as range value and base for calculations (flix, bonus, hourly, dayly, monthly, …)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "")
      .AddAttribute(5, "SalaryBase", "SalaryBase", "integer", 0, 0, false, false, "", "", "", "")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "")

      .AddReference("refSalaryType2SalaryBase", EMyReferenceType::range, "SalaryBase", "has values", "n : 1", { 2 }, "range value for the salary base, used for different kinds of calculations", { {5,1} })

      ;

   dictionary.AddTable("TaxClasses", EMyEntityType::table, "TaxClasses", "dbo", "TaxClasses", "myHR", "System/HR", "SQL", "tax class in germany to use it with employees (not a range, but very similar)")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with tax classes, used as key and attribute for relationships")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique identification number for a record in this domain with tax class types, used in the programm to indentify this tax class, as selection in comboboxes and such")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, false, false, "", "", "", "unique abbreviation of a tax class entity in the domain, used in overviews and reports")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "description for this tax class, used for the program")
      .AddAttribute(5, "Coupled", "Coupled", "bool", 0, 0, true, false, "", "", "", "Indicator whether this tax class applies to married couples or registered civil partnerships")
      .AddAttribute(6, "UrgentValue", "UrgentValue", "bool", 0, 0, false, false, "", "", "", "boolean value that makes this entity of tax classes to a system value (cannot be changed as it is used directly by the program)")

      .AddIndex("uk_TaxClasses_Denotation", EMyIndexType::key, "unique / representative official denotation of the tax class as key canditate for this entity", { { 2, true } })
      .AddIndex("uk_TaxClasses_Abbreviation", EMyIndexType::key, "unique / representative official abbreviation of the tax class as key canditate for this entity", { { 3, true } })

      .AddRangeValue("INSERT INTO TaxClasses (ID, Denotation, Abbreviation, Description, Coupled, UrgentValue) VALUES\n"
                     "    (1, 'Steuerklasse 1', 'I', 'Dies ist die Standardsteuerklasse für alleinstehende Personen ohne Kinder.', 0, 1), \n"
                     "    (2, 'Steuerklasse 2', 'II', 'Diese Steuerklasse ist für Alleinerziehende mit mindestens einem Kind gedacht.', 0, 1), \n"
                     "    (3, 'Steuerklasse 3', 'III', 'Diese Klasse ist für verheiratete oder in einer eingetragenen Lebenspartnerschaft lebende Personen, wenn der Ehe- oder Lebenspartner in Steuerklasse V eingestuft ist oder keine Einkünfte hat.', 1, 1), \n"
                     "    (4, 'Steuerklasse 4', 'IV', 'Dies ist die Standardsteuerklasse für verheiratete oder in einer eingetragenen Lebenspartnerschaft lebende Personen, wenn beide Partner ein ähnliches Einkommen haben.', 1, 1), \n"
                     "    (5, 'Steuerklasse 5', 'V', 'Diese Klasse wird für den Ehe- oder Lebenspartner verwendet, wenn der andere Partner in Steuerklasse III eingestuft ist.', 1, 1), \n"
                     "    (6, 'Steuerklasse 6', 'VI', 'Diese Klasse wird verwendet, wenn eine Person mehrere Jobs hat oder zusätzliche Einkünfte aus einer geringfügigen Beschäftigung hat.', 0, 1), \n"
                     "    (7, 'Ausland', 'A', 'Person wohnt im Ausland, Steuer individuell bestimmen ', 0, 1), \n"
                     "    (8, 'unbekannt', '-', 'Steuerklasse noch unbekannt, unbedingt nachfragen, vorläufig als Steuerklasse 6 abrechnen', 0, 1)")
      ;


   dictionary.AddTable("WD_Holidays", EMyEntityType::table, "WD_Holidays", "dbo", "WD_Holidays", "myHR", "System/HR", "SQL", "entities with public holidays, in relation to working days table to determine non working days")
      .AddAttribute(1, "CalendarDay", "CalendarDay", "date", 0, 0, true, true, "", "", "", "calendar day on which the holiday is")
      .AddAttribute(2, "Donation", "Donation", "varchar", 50, 0, true, false, "", "", "", "specification of the holiday as text")
      .AddAttribute(3, "Share", "Share", "integer", 0, 0, true, false, "", "", "", "share of the holiday (addition for later extension, e.g. holy eve as a half holiday)")
      .AddAttribute(4, "Description", "Description", "text", 0, 0, false, false, "", "", "", "additional, more detailed description of the holiday. can use as hint in programm or reports")

      .AddReference("refWD_Holidays_Workdays", EMyReferenceType::assoziation, "WD_Workdays", "is free", "1 : 1", { }, "assoziation of a holiday to the working days table", { {1,1} })

      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function
      .AddPostConditions("CREATE FUNCTION [dbo].[GetIsHoliday](@caldate AS DATE) RETURNS INTEGER\n"
         "BEGIN\n"
         "   DECLARE @retval INTEGER;\n"
         "   SELECT @retval = COUNT(*)\n"
         "   FROM dbo.WD_Holidays\n"
         "   WHERE CalendarDay = @caldate;\n"
         "   RETURN @retval;\n"
         "END")
      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function

      .AddCleanings("DROP FUNCTION [dbo].[GetIsHoliday]")

      ;

   dictionary.AddTable("WD_Months", EMyEntityType::range, "WD_Months", "dbo", "WD_Months", "myHR", "System/HR", "SQL", "domain with months for human resources, actually used for working time, later for processes too. This range make it possible additional informations to adding later. This isn't a typical range value.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "BETWEEN 1 AND 12", "", "", "unique identification number for a record in this domain with monthes for human resources")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 50, 0, true, false, "", "", "", "unique denotation for a record in this domain with months, used in the programm to indentify this month, as selection in comboboxes and such")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 10, 0, true, false, "", "", "", "unique abbreviation of a month in the domain, used in overviews and reports")
      .AddAttribute(4, "Quarter", "Quarter", "unsigned", 0, 0, true, false, "BETWEEN 1 AND 4", "", "", "unsigned value with the number auf the quarter, values in 1 - 4")

      .AddIndex("uk_WD_Months_Denotation", EMyIndexType::key, "unique / representative denotation for this month, use in selections or to display", { { 2, true } })
      .AddIndex("uk_WD_Months_Abbreviation", EMyIndexType::key, "unique / representative abbreviation for this month, use in selections or to display", { { 3, true } })

      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function
      .AddPostConditions("CREATE FUNCTION [dbo].[GetQuarterOfMonth](@id AS INTEGER) RETURNS VARCHAR(10)\n"
         "BEGIN\n"
         "   DECLARE @retval VARCHAR(10);\n"
         "   SELECT @retval = Quarter\n"
         "   FROM dbo.WD_Months\n"
         "   WHERE ID = @id;\n"
         "   RETURN @retval;\n"
         "END")
      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function


      .AddRangeValue("INSERT INTO WD_Months (ID, Denotation, Abbreviation, Quarter) VALUES\n"
                     "    (1, 'Januar', 'Jan', 1), \n"
                     "    (2, 'Februar', 'Feb', 1), \n"
                     "    (3, 'März', 'Mär', 1), \n"
                     "    (4, 'April', 'Apr', 2), \n"
                     "    (5, 'Mai', 'Mai', 2), \n"
                     "    (6, 'Juni', 'Jun', 2), \n"
                     "    (7, 'Juli', 'Jul', 3), \n"
                     "    (8, 'August', 'Aug', 3), \n"
                     "    (9, 'September', 'Sep', 3), \n"
                     "    (10, 'Oktober', 'Okt', 4), \n"
                     "    (11, 'November', 'Nov', 4), \n"
                     "    (12, 'Dezember', 'Dez', 4)")

      .AddCleanings("DROP FUNCTION [dbo].[GetQuarterOfMonth]")
      ;

   dictionary.AddTable("WD_NonWorking", EMyEntityType::table, "WD_NonWorking", "dbo", "WD_NonWorking", "myHR", "System/HR", "SQL", "entity set with non working days. Extensions possible, responsible for this table is HR.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "attribute as foreign key from an attribute ID of a employee entity to who as idle this time")
      .AddAttribute(2, "StartAt", "StartAt", "date", 0, 0, true, true, "", "", "", "date when the non working time starts as foreign key from the wokdays table")
      .AddAttribute(3, "ClosingAt", "ClosingAt", "date", 0, 0, false, false, "", "", "", "date when the non working time finishing. Value must exists in workdays, but isn't in the key. May be open")
      .AddAttribute(4, "Reason", "Reason", "integer", 0, 0, true, false, "BETWEEN 0 AND 6", "", "", "reason of non working time, there exist a table, but values are interpreted through the applications")
      .AddAttribute(5, "Notes", "Notes", "text", 0, 0, false, false, "", "", "", "free notices to the non-working time")

      .AddReference("refNonWorking2Employee", EMyReferenceType::composition, "Employees", "idled", "1 : n", { }, "part of relationship to idle times / times of non working", { {1,1} })
      .AddReference("refNonWorking2Workday_Start", EMyReferenceType::assoziation, "WD_Workdays", "is at", "n : 1", { }, "assoziation between the non-working time and the working days for the begin", { {2,1} })
      .AddReference("refNonWorking2Workday_Finishing", EMyReferenceType::assoziation, "WD_Workdays", "is at", "n : 1", { }, "assoziation between the non-working time and the working days for the end", { {3,1} })
      .AddReference("refNonWorking2Reason", EMyReferenceType::range, "ReasonNonWorking", "has values", "n : 1", { 2 }, "range value who describe the reason for the idle time", { {4,1} })

      ;

   dictionary.AddTable("WD_Weekdays", EMyEntityType::range, "WD_Weekdays", "dbo", "WD_Weekdays", "myHR", "System/HR", "SQL", "domain for days of week, used for table working time. Possible to extent with informations. In area HR only")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "unique identification number for a record in this domain with weekdays for human resources")
      .AddAttribute(2, "Denotation", "Denotation", "varchar", 20, 0, true, false, "", "", "", "unique denation for this weekday ")
      .AddAttribute(3, "Abbreviation", "Abbreviation", "varchar", 5, 0, true, false, "", "", "", "unique abbreviation for this weekday ")
      .AddAttribute(4, "Workday", "Workday", "bool", 0, 0, true, false, "", "", "", "boolean value to control the work time, true if this weekday usual a workday")

      .AddIndex("uk_WD_Weekdays_Denotation", EMyIndexType::key, "unique / representative denotation for this weekday, use in selections or to display", { { 2, true } })
      .AddIndex("uk_WD_Weekdays_Abbreviation", EMyIndexType::key, "unique / representative abbreviation for this weekdays, use in selections or to display", { { 3, true } })

      .AddRangeValue("INSERT INTO WD_Weekdays (ID, Denotation, Abbreviation, Workday) VALUES\n"
                     "    (0, 'Montag', 'Mo', 1), \n"
                     "    (1, 'Dienstag', 'Di', 1), \n"
                     "    (2, 'Mittwoch', 'Mi', 1), \n"
                     "    (3, 'Donnerstag', 'Do', 1), \n"
                     "    (4, 'Freitag', 'Fr', 1), \n"
                     "    (5, 'Sonnabend', 'Sa', 0), \n"
                     "    (6, 'Sonntag', 'So', 0)")

      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function
      .AddPostConditions("CREATE FUNCTION [dbo].[GetIsWorkday](@id AS INTEGER) RETURNS SMALLINT\n"
         "BEGIN\n"
         "   DECLARE @retval SMALLINT;\n"
         "   SELECT @retval = Workday\n"
         "   FROM dbo.WD_Weekdays\n"
         "   WHERE ID = @id;\n"
         "   RETURN @retval;\n"
         "END")
      .AddPostConditions("GO") // SQL Server required a new stack of statements before creating a function

      .AddCleanings("DROP FUNCTION [dbo].[GetIsWorkday]")

      ;

   dictionary.AddTable("WD_Workdays", EMyEntityType::table, "WD_Workdays", "dbo", "WD_Workdays", "myHR", "System/HR", "SQL", "domain with all days, additional informations and as owner of all recorded working time and non-working times")
      .AddAttribute(1, "CalendarDay", "CalendarDay", "date", 0, 0, true, true, "", "", "", "calendar day. is used as a key in all day-dependent time accounting tables. it is the basis for the calculated fields that are provided also")
      .AddAttribute(2, "CalendarWeekday", "CalendarWeekday", "integer", 0, 0, true, false, "", "", "", "day in the calendar week (values between 0 .. 6). The field referencing to the table with weekdays to get rules. set as a data element to avoid calculations, possible server settings and deviations from various standards")
      .AddAttribute(3, "CalendarWeek", "CalendarWeek", "integer", 0, 0, true, false, "", "", "", "calendar week. set as a data element to avoid calculations, possible server settings and deviations from various standards")
      .AddAttribute(4, "CalendarYear", "CalendarYear", "integer", 0, 0, true, false, "", "", "(YEAR(CalendarDay))", "calculated field for the year of the calendar day")
      .AddAttribute(5, "CalendarMonth", "CalendarMonth", "integer", 0, 0, true, false, "", "", "(MONTH(CalendarDay))", "calculated field for the year of the calendar day, persistent for the relationship to the table WD_Months")
      .AddAttribute(6, "CalendarDayInWeek", "CalendarDayInWeek", "integer", 0, 0, true, false, "", "", "(DAY(CalendarDay))", "calculated field for the year of the calendar day")
      .AddAttribute(7, "CalendarDayInYear", "CalendarDayInYear", "integer", 0, 0, true, false, "", "", "DATEPART(DAYOFYEAR, CalendarDay)", "calculated field for the year of the calendar day")
      .AddAttribute(8, "CalendarQuarter", "CalendarQuarter", "integer", 0, 0, true, false, "", "", "[[dbo].[GetQuarterOfMonth](MONTH(CalendarDay))]", "calculated field, use relationship to WD_Months and the function GetQuarterOfMonth to get the quarter")
      .AddAttribute(9, "Workday", "Workday", "bool", 0, 0, true, false, "", "", "[IIF(dbo.GetIsWorkday(CalendarWeekday) = 1 AND dbo.GetIsHoliday(CalendarDay) = 0, 1, 0)]", "boolean value which determin the day as work day, using functions of WD_Weekdays and WD_Holidays")

      .AddReference("refWD_Workdays2WeekDay", EMyReferenceType::range, "WD_Weekdays", "has values", "n : 1", { 2 }, "range value for the calendar weekday", { {2,1} })
      .AddReference("refWD_Workdays2Month", EMyReferenceType::range, "WD_Months", "has values", "n : 1", { 2 }, "range value for the calendar month", { {5,1} })
 
      .AddIndex("idxWD_Workdays_Years_Month", EMyIndexType::undefined, "access path for search year and month for reports", { { 4, true }, { 5, true } })

      ;

   dictionary.AddTable("WorkingTime", EMyEntityType::table, "WorkingTime", "dbo", "WorkingTime", "myHR", "System/HR", "SQL", "entity with the working times for an employee, as a composition for these.")
      .AddAttribute(1, "ID", "ID", "integer", 0, 0, true, true, "", "", "", "attribute as foreign key from an attribute ID of a employee entity to who as worked these time")
      .AddAttribute(2, "StartingTime", "StartingTime", "datetime", 0, 0, true, true, "", "", "", "date and time at which this work block started as timestamp")
      .AddAttribute(3, "ClosingTime", "ClosingTime", "datetime", 0, 0, false, false, "[convert(date, ClosingTime) = DayOfWork and ClosingTime > StartingTime]", "{ }", "", "date and time at which this block of work finished as timestamp")
      .AddAttribute(4, "Processed", "Processed ", "bool", 0, 0, true, false, "", "", "", "boolean indicator that specifies whether this time booking has already been charged")
      .AddAttribute(5, "ProcessedAt", "ProcessedAt", "datetime", 0, 0, false, false, "", "", "", "time at which this data record was settled and posted")
      .AddAttribute(6, "DayOfWork", "DayOfWork", "date", 0, 0, false, false, "", "", "{convert(date, StartingTime)}", "calculated day to the timepoint where work started")

      .AddReference("refWorkTime2Employee", EMyReferenceType::composition, "Employees", "worked", "1 : n", { }, "part of relationship of work times to the employees", { {1,1} })
      ;


   dictionary.FindTable("Person")
      .AddDescription("The table \"Person\" serves as an abstraction of the \"Employees\" table, aiming to separate general attributes "
                      "from specific employee-related ones. This abstraction facilitates the derivation of other types of individuals in the future.")

      .AddDescription("The \"Employees\" table inherits (is-a relationship) from the \"Person\" table, utilizing the primary key of \"Person\" as a "
                      "foreign key.This establishes a referential relationship, ensuring data integrity and consistency between the two tables.")

      .AddDescription("By employing this design, future extensions or modifications to accommodate different types of individuals beyond employees "
                      "become feasible without compromising the integrity of the database structure.")
      ;

   dictionary.FindTable("Employees")
      .AddDescription("This table, together with the Person table, forms the basis of this sample application. The aim is to implement a time "
                      "accounting system for various situations that takes into account not only the working time rules on weekdays but also "
                      "time off (vacation, illness, etc.). This is the basis for time recording and payroll accounting. All other areas are "
                      "based around this.")
      ;

   dictionary.FindTable("WorkingTime")
      .AddDescription("This class uses the ID of the employee as a foreign key in the database and extends it by the time at which a work segment begins.\n"
                      "In the second phase of the project, this class / table is to be extended so that relationships can be established with projects / "
                      "project parts and so that the working time can be related to project activities.")
      .AddComment("the relationship with the project tasks is similar as the relationship to the employee, but without existence dependency.");

   }
