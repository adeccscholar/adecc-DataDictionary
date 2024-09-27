/** \file
   \brief Testprogramm für die neue Metadatenverwaltung im Projekt "adecc Scholar"
   \details Nutzen der Metadaten um einzelne Aufgaben zu erfüllen
   \version 1.0
   \since Version 1.0
   \authors Volker Hillmann (VH)
   \date 29.01.2024 VH erstellt
   \date 31.01.2024 VH Trennung und Aufteilung in Definition und Implementierung
   \date 04.02.2024 VH Dokumentation ergänzt, Bereich für Indizes ergänzt, Referenzen typisiert und Beschreibung / Notizen ergänzt
                    Dictionary in eigene Klasse TDictionary_Berlin ausgelagert
   \copyright copyright &copy; 2024. Alle Rechte vorbehalten.
   This project is released under the MIT License.
*/

#include "Berlin_Dictionary.h"
#include "Test_Dictionary.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <tuple>
#include <map>
#include <vector>
#include <format>
#include <stdexcept>

using namespace std::string_literals;

// method for test only geodata model
void Test(TMyDictionary const& dictionary) {
   std::cout << dictionary.FindDataType("bigint").SourceType() << "\n\n";

   dictionary.FindTable("Berlin").Funktion("ref_test"s, 1u, 3u, 5u, "Berlin"s, 1u, 7u, 6u);

   for (auto const& [key, val] : dictionary.DataTypes()) std::cout << key << "\n";
   std::cout << '\n';

   std::cout << dictionary.FindTable("Berlin").FindAttribute("Street_Date").DataType() << "\n\n";
   } 


int main() {
   try {
      TDictionary_Test dict_data;
  
      /// defining the paths for the output during metadata processing
      /*
      dict_data.Dictionary().SourcePath("D:\\Test\\Persons\\Src");
      dict_data.Dictionary().SQLPath("D:\\Test\\Persons\\SQL");
      */


      dict_data.Dictionary().SourcePath("D:\\Projekte\\GitHub\\Test_Metadata_Creator\\src");
      dict_data.Dictionary().SQLPath("D:\\Projekte\\GitHub\\Test_Metadata_Creator\\SQL");

      dict_data.Dictionary().PathToPersistence("Persistence");
      dict_data.Dictionary().DocPath("D:\\Projekte\\GitHub\\Test_Metadata_Creator\\Documentation");
      dict_data.Dictionary().IDLPath("D:\\Projekte\\GitHub\\Test_Metadata_Creator\\IDL");
      dict_data.Dictionary().CorbaPath("D:\\Projekte\\GitHub\\Test_Metadata_Creator\\src\\Corba");

      constexpr bool boTest = true;

      if(boTest) {
         /*
         for (auto const& className : { "Employees", "WD_NonWorking" }) {
            auto test1 = dict_data.Dictionary().FindTable(className).GetPrecursors(false);
            std::cout << std::format("\nPrecursors for class {}:\n", className);
            std::ranges::for_each(test1, [](auto t) { std::cout << t << "\n";  });
            auto test2 = dict_data.Dictionary().FindTable(className).GetSuccessors(false);
            std::cout << std::format("\nSuccessors for class {}:\n", className);
            std::ranges::for_each(test2, [](auto t) { std::cout << t << "\n";  });
            }
         */
         dict_data.Dictionary().Test();
         }
      else {
         dict_data.Dictionary().Test();


         dict_data.Dictionary().Create_All(std::cout, std::cerr);
         std::cout << "\n\ngenerator finished without error\n";
         }
      }
   catch(std::exception& ex) {
      std::cerr << ex.what() << '\n';
      }
}

