#include "Berlin_Dictionary.h"

#include <sstream>

TDictionary_Berlin::TDictionary_Berlin() : dictionary("geographic data example") {
   std::ostringstream os;

   os << "This project is about testing and demonstrating the possibilities of programming with metadata.\n"
      << "There is no real benefit.The data is provided from public data sources with the \"OpenData Lizenz Deutschland\".";

   dictionary.Description("test project for the metadata generator if adecc Scholar with the geographic data of Berlin");
   dictionary.Comment(os.str());

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

   dictionary.AddTable("Berlin", EMyEntityType::table, "Berlin_olddata", "dbo", "Berlin", "myDataOld", "System", "SQL", "Tabelle mit den Berliner Adresspunkten")
      .AddAttribute(1, "State_ID", "Land_ID", "integer", 0, 0, false, false, "Identifikationsnummer (NUTS) für das Bundesland")
      .AddAttribute(2, "District_ID", "Bezirk_ID", "integer", 0, 0, false, false, "")
      .AddAttribute(3, "Neighborhood_ID", "Ortsteil_ID", "integer", 0, 0, false, false, "")
      .AddAttribute(4, "Street_ID", "Strasse_ID", "integer", 0, 0, false, false, "")
      .AddAttribute(5, "State", "Land", "varchar", 75, 0, false, false, "")
      .AddAttribute(6, "City", "Stadt", "varchar", 75, 0, false, false, "")
      .AddAttribute(7, "District", "Bezirk", "varchar", 75, 0, false, false, "")
      .AddAttribute(8, "Neighborhood", "Ortsteil", "varchar", 75, 0, false, false, "")
      .AddAttribute(9, "Street", "Strasse", "varchar", 75, 0, false, false, "")
      .AddAttribute(10, "Postal_Code", "PLZ", "varchar", 5, 0, false, false, "")
      .AddAttribute(11, "House_Number", "HNR", "integer", 0, 0, false, false, "")
      .AddAttribute(12, "Address_Addendum", "ADZ", "varchar", 5, 0, false, false, "")
      .AddAttribute(13, "Latitude", "Latitude", "double", 0, 0, false, false, "")
      .AddAttribute(14, "Longitude", "Longitude", "double", 0, 0, false, false, "")
      .AddAttribute(15, "Address_Date", "Adr_Datum", "date", 0, 0, false, false, "")
      .AddAttribute(16, "Street_Date", "Strasse_Datum", "date", 0, 0, false, false, "")
      .AddAttribute(17, "Quality", "Qualitaet", "varchar", 100, 0, false, false, "")
      .AddAttribute(18, "Address_Type", "Adr_Typ", "varchar", 100, 0, false, false, "")
      .AddAttribute(19, "HKO_ID", "HKO_ID", "varchar", 20, 0, false, false, "")
      
      .AddReference("refBerlin", EMyReferenceType::assoziation, "Berlin", "Tabelle mit den ursprünglichen Importdaten von Berlin", { {1,1}, {3,7}, {5,5} })
      
      .AddIndex("idXBerlinStrasse", EMyIndexType::undefined, "Testindex für die Tabelle", { { 1, true }, { 2, false }, { 3, false } })
      ;

}