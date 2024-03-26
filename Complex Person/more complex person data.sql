/* ------------------------------------------------------------------------------------------------------------------
    Complex Database Example with Conditional Dependencies in C++ using SFINAE
	-----------------------------------------------------------------------------------------------------------------
    Overview:
    This documentation presents a complex database example that involves conditional dependencies, which can 
	be implemented in C++ through Substitution Failure Is Not An Error (SFINAE) techniques.
	-----------------------------------------------------------------------------------------------------------------
    Complex Database Example:
    The database scenario includes intricate relationships between entities, where certain dependencies are 
	conditionally established based on specific criteria. For instance, consider a database managing different 
	information about persons in a company. Depending on whether a person is a natural person or legal entity, 
	different sets of attributes may need to be stored.
	
	These conditional dependencies create a complex structure within the database schema.
	-----------------------------------------------------------------------------------------------------------------
    Implementation in C++ using SFINAE:
    Substitution Failure Is Not An Error (SFINAE) is a powerful feature in C++ template metaprogramming that 
	allows for the selection of different function templates or classes based on compile-time conditions. In 
	the context of our database example, SFINAE can be leveraged to handle conditional dependencies efficiently. 
	By using template specialization and type traits, we can define different database entities and their 
	attributes based on compile-time conditions. 
	
	This enables us to create a flexible and scalable database framework capable of accommodating complex 
	conditional relationships.
	-----------------------------------------------------------------------------------------------------------------
    Integration with Triggers:
    Furthermore, managing conditional dependencies in this complex database example necessitates the use of 
	triggers. Unlike standard SQL and simple integrity constraints, triggers allow for the execution of custom 
	logic in response to database changes. By employing triggers, developers can enforce complex business rules, 
	maintain data integrity, and handle intricate dependencies dynamically.
	-----------------------------------------------------------------------------------------------------------------
    Conclusion:
    In conclusion, the combination of a complex database example with conditional dependencies, its implementation 
	in C++ using SFINAE, and integration with triggers offers a robust solution for managing intricate data 
	structures. Through careful design and utilization of advanced C++ features, developers can create highly 
	adaptable database systems capable of meeting diverse requirements and evolving business needs.

   ----------------------------------------------------------------------------------------------------------------- */

USE Test

/* -----------------------------------------------------------------------------------------------------------------
   clean database before creating the structure 
   ----------------------------------------------------------------------------------------------------------------- */

DROP TRIGGER IF EXISTS trgNaturalPersonUpdate
DROP TRIGGER IF EXISTS trgNaturalPersonDelete
DROP TRIGGER IF EXISTS trgLegalEntityUpdate
DROP TRIGGER IF EXISTS trgLegalEntityDelete
DROP TRIGGER IF EXISTS trgRealPersonCheck
DROP TABLE IF EXISTS RealPerson
DROP TABLE IF EXISTS LegalEntity
DROP TABLE IF EXISTS NaturalPerson
DROP TABLE IF EXISTS EntityOrPerson
DROP FUNCTION IF EXISTS Get_Salutation_Text
DROP FUNCTION IF EXISTS GetAge
DROP TABLE IF EXISTS CorporateForm
DROP TABLE IF EXISTS Salutation
DROP TABLE IF EXISTS KindOfPerson

/* ------------------------------------------------------------------------------------------------------------
   table name: KindOfPerson
   this table serves as a domain to differentiate between natural and legal persons.
   ------------------------------------------------------------------------------------------------------------
   ID:         unique identifier for each type of person.
   Denotation: describes the type of person (natural or legal).
   ------------------------------------------------------------------------------------------------------------
   ID is primary key, Denotation key candidate 
   only 2 tuple allowed with the ID IN (1, 2)
   1 - natural
   2 - legal
   ------------------------------------------------------------------------------------------------------------ */

CREATE TABLE KindOfPerson (
   ID INTEGER NOT NULL,
   Denotation VARCHAR(30) NOT NULL
   )

ALTER TABLE KindOfPerson ADD CONSTRAINT pkKindOfPerson PRIMARY KEY (ID)
ALTER TABLE KindOfPerson ADD CONSTRAINT ukKindOfPerson_Denotation UNIQUE (Denotation)
ALTER TABLE KindOfPerson ADD CONSTRAINT ckKindOfPerson_ID CHECK (ID IN (1,2))

INSERT INTO KindOfPerson (ID, Denotation) VALUES (1, 'natürliche Person'), (2, 'juristische Person (Entity)')

/* ------------------------------------------------------------------------------------------------------------
   table name: EntityOrPerson 
   this table serves as the base for categorizing entities as either natural persons or legal entities. 
   it includes references to the "KindOfPerson" table to determine the type of entity.

   this table establishes the foundational structure for distinguishing between natural persons and 
   legal entities within the database. it acts as the base for creating specialized tables for each 
   entity type, maintaining an 'is-a' relationship.
   ------------------------------------------------------------------------------------------------------------
   ID:     unique identifier for each entity.
   KindOf: integer value referencing the "KindOfPerson" table to specify whether the entity is a natural 
           person or a legal entity.
   ------------------------------------------------------------------------------------------------------------
   primary key attribute ID and a additional key candidate with ID and KindOf as anchor for controlled 
   references with a specific kind of person
   ------------------------------------------------------------------------------------------------------------ */

CREATE TABLE EntityOrPerson (
   ID INTEGER NOT NULL,
   KindOf INTEGER NOT NULL
   )

ALTER TABLE EntityOrPerson ADD CONSTRAINT pkPerson PRIMARY KEY (ID)
ALTER TABLE EntityOrPerson ADD CONSTRAINT ukPerson_Key UNIQUE (ID, KindOf)
ALTER TABLE EntityOrPerson ADD CONSTRAINT refPerson_KindOfPerson FOREIGN KEY (KindOf) REFERENCES KindOfPerson (ID)

GO

/* -------------------------------------------------------------------------------------------------------------
   function name: GetAge
   Function for Calculating Age of Natural Person

   user-defined function in SQL Server designed to calculate the age of a natural person based on their 
   date of birth. This function, named "GetAge", takes a date of birth as input and returns the person's 
   age as an integer.

   The "GetAge" function employs SQL Server's built-in date manipulation functions to accurately determine 
   the age of a person. It utilizes the DATEADD and DATEDIFF functions to calculate the time difference 
   between the current date (GETDATE()) and the provided birth date. The function accounts for cases where 
   the current date has not yet reached the person's birthday within the current calendar year.
   ------------------------------------------------------------------------------------------------------------- */

CREATE FUNCTION GetAge (@birth AS DATE) RETURNS INT
BEGIN
   RETURN IIF(DATEADD(YEAR, DATEDIFF(YEAR, @birth, GETDATE()), @birth) > GETDATE(), DATEDIFF(YEAR, @birth, GETDATE()) - 1, DATEDIFF(YEAR, @birth, GETDATE()));
END
GO

/* ------------------------------------------------------------------------------------------------------------
   table name: Salutation
   Salutation Table for Natural Persons

   this table serves as a value range table for salutations used in addressing natural persons. The table 
   contains unique identifiers for different salutations, along with their corresponding denotations.
   ------------------------------------------------------------------------------------------------------------
   ID:         an integer field serving as the unique identifier for each salutation.
   Denotation: a string field storing the actual salutation denotation.
   ------------------------------------------------------------------------------------------------------------
   The "Salutation" table can be utilized in conjunction with other tables containing natural person data 
   to provide appropriate salutations based on individual characteristics such as gender, professional 
   title, or cultural norms. It enables standardized and respectful communication with individuals by 
   selecting the appropriate salutation based on predefined denotations.

   In conclusion, the "Salutation" table serves as a valuable reference for managing salutations used 
   in addressing natural persons. By storing a range of common salutations along with their unique 
   identifiers, the table facilitates consistent and respectful communication practices within various 
   applications and systems.
   ------------------------------------------------------------------------------------------------------------ */

CREATE TABLE Salutation (
   ID INTEGER NOT NULL,
   Denotation VARCHAR(20) NOT NULL
   )

ALTER TABLE Salutation ADD CONSTRAINT pkSalutation PRIMARY KEY (ID)
ALTER TABLE Salutation ADD CONSTRAINT ukSalutation_Denotation UNIQUE (Denotation)

INSERT INTO Salutation (ID, Denotation) VALUES (1, 'Herr'), (2, 'Frau')

GO
/* ------------------------------------------------------------------------------------------------------------
   function name: GetAge
   Function for Retrieving Salutation Text

   this function is designed to retrieve the text representation of a salutation based on its unique identifier.

   The "Get_Salutation_Text" function is a user-defined scalar function in SQL Server. It takes an integer 
   parameter representing the ID of a salutation and returns the corresponding text representation of that 
   salutation. The function retrieves the salutation text from the "Salutation" table based on the provided ID.
   ------------------------------------------------------------------------------------------------------------ */
CREATE FUNCTION Get_Salutation_Text (@id AS INTEGER) RETURNS VARCHAR(20)
BEGIN
   DECLARE @retval VARCHAR(20);
   SELECT @retval = Denotation
   FROM dbo.Salutation
   WHERE ID = @id;
   RETURN @retval;
END

GO
/* ------------------------------------------------------------------------------------------------------------
   table name: NaturalPerson
   this table is designed to store information about natural persons within a database system. The table 
   structure facilitates the management of personal data, including names, salutations, birth dates, 
   and an age calculations.

   The "NaturalPerson" table captures essential details about natural persons, such as their names, birth dates, 
   and salutations. Additionally, it includes computed columns for the person's full name, salutation text, and 
   age, providing convenient access to derived information.

   Fields:
   ID (INTEGER): Unique identifier for each natural person.
   KindOf (INTEGER): Specifies the type of person (natural person). Is 1 (checked with a condition)
   Name (VARCHAR(25)): Last name of the natural person.
   FirstName (VARCHAR(25)): First name of the natural person.
   Salutation (INTEGER): Salutation ID referencing the "Salutation" table. (referenced to table Salutation)
   BirthDate (DATE): Date of birth of the natural person.

   Computed Columns:
   Fullname: Computed column concatenating the last name and first name.
   Salutation_Text: Computed column retrieving the text representation of the salutation using a user-defined function.
   Age: Computed column calculating the age of the natural person based on the birth date using a user-defined function.
   ------------------------------------------------------------------------------------------------------------ */
CREATE TABLE NaturalPerson (
   ID INTEGER NOT NULL,
   KindOf INTEGER DEFAULT 1 NOT NULL,
   Name VARCHAR(25) NOT NULL,
   FirstName VARCHAR(25),
   Salutation INTEGER,
   BirthDate DATE,
   Fullname AS Name + ', ' + FirstName,
   Salutation_Text AS [dbo].Get_Salutation_Text(Salutation),
   Age AS [dbo].GetAge(BirthDate)
   )

ALTER TABLE NaturalPerson ADD CONSTRAINT pkNaturalPerson PRIMARY KEY (ID)
ALTER TABLE NaturalPerson ADD CONSTRAINT ckNaturalPerson_KindOf CHECK (KindOf = 1)
ALTER TABLE NaturalPerson ADD CONSTRAINT refNaturalPerson_Person FOREIGN KEY (ID, KindOf) REFERENCES EntityOrPerson (ID, KindOf)
ALTER TABLE NaturalPerson ADD CONSTRAINT refNaturalPerson_Salutation FOREIGN KEY (Salutation) REFERENCES Salutation (ID)

CREATE INDEX idxNaturalPerson_Name ON NaturalPerson (Name, FirstName)

-- ------------------------------------------------------------------------------------------------------

CREATE TABLE CorporateForm (
   ID INTEGER NOT NULL,
   Denotation VARCHAR(20) NOT NULL,
   EntityAsPerson TINYINT NOT NULL CHECK (EntityAsPerson IN (0, 1))
   )

ALTER TABLE CorporateForm ADD CONSTRAINT pkCorporateForm PRIMARY KEY (ID)
ALTER TABLE CorporateForm ADD CONSTRAINT ukCorporateForm_Denotation UNIQUE (Denotation)
GO

INSERT INTO CorporateForm (ID, Denotation, EntityAsPerson) VALUES (1, 'GmbH', 1), (2, 'AG', 1), (3, 'UG', 1), (4, 'OHG', 0), (5, 'KG', 0), (6, 'GmbH & Co. KG', 1)

CREATE TABLE LegalEntity (
   ID INTEGER NOT NULL,
   KindOf INTEGER DEFAULT 2 NOT NULL,
   Name VARCHAR(25) NOT NULL,
   CorporateForm INTEGER NOT NULL
   )

ALTER TABLE LegalEntity ADD CONSTRAINT pkLegalEntity PRIMARY KEY (ID)
ALTER TABLE LegalEntity ADD CONSTRAINT ckLegalEntity_KindOf CHECK (KindOf = 2)
ALTER TABLE LegalEntity ADD CONSTRAINT refLegalEntity_Person FOREIGN KEY (ID, KindOf) REFERENCES EntityOrPerson (ID, KindOf)
ALTER TABLE LegalEntity ADD CONSTRAINT refLegalEntity_CorporateForm FOREIGN KEY (CorporateForm) REFERENCES CorporateForm (ID)

CREATE Table RealPerson (
   ID INTEGER NOT NULL,
   KindOf INTEGER NOT NULL
   )

ALTER TABLE RealPerson ADD CONSTRAINT pkRealPerson PRIMARY KEY (ID)
ALTER TABLE RealPerson ADD CONSTRAINT reRealPerson_Person FOREIGN KEY (ID, KindOf) REFERENCES EntityOrPerson (ID, KindOf)
-- ALTER TABLE EntityOrPerson ADD CONSTRAINT refRealPerson_KindOfPerson FOREIGN KEY (KindOf) REFERENCES KindOfPerson (ID)

GO

CREATE TRIGGER trgNaturalPersonDelete ON NaturalPerson
INSTEAD OF DELETE
AS
BEGIN
    IF EXISTS (SELECT 1 FROM RealPerson WHERE KindOf = 1 AND ID IN (SELECT ID FROM deleted))
    BEGIN
        RAISERROR ('Entry in NaturalPerson cannot be deleted while it is referenced by RealPerson.', 16, 1);
        ROLLBACK TRANSACTION;
        RETURN;
    END;
    ELSE
    BEGIN
        DELETE FROM NaturalPerson WHERE ID IN (SELECT ID FROM deleted);
    END;
END;
GO

CREATE TRIGGER trgNaturalPersonUpdate ON NaturalPerson
AFTER UPDATE
AS
BEGIN
    IF EXISTS (SELECT 1 FROM inserted i JOIN RealPerson rp ON i.ID = rp.ID)
    BEGIN
        IF EXISTS (SELECT 1 FROM inserted i WHERE NOT EXISTS (SELECT 1 FROM deleted d WHERE d.ID = i.ID) )
        BEGIN
            RAISERROR ('Updates of ID and KindOf in NaturalPerson are not allowed as they are referenced by RealPerson.', 16, 1);
            ROLLBACK TRANSACTION; -- optional, to recover the transaction
        END;
    END;
END;
GO


CREATE TRIGGER trgLegalEntityDelete ON LegalEntity
INSTEAD OF DELETE
AS
BEGIN
    IF EXISTS (SELECT 1 FROM LegalEntity WHERE KindOf = 2 AND ID IN (SELECT ID FROM deleted))
    BEGIN
        RAISERROR ('Entry in LegalEntity cannot be deleted because it is referenced by RealPerson.', 16, 1);
        ROLLBACK TRANSACTION;
        RETURN;
    END;
    ELSE
    BEGIN
        DELETE FROM LegalEntity WHERE ID IN (SELECT ID FROM deleted);
    END;
END;
GO

CREATE TRIGGER trgLegalEntityUpdate ON NaturalPerson
AFTER UPDATE
AS
BEGIN
    IF EXISTS (SELECT 1 FROM inserted i JOIN RealPerson rp ON i.ID = rp.ID)
    BEGIN
        IF EXISTS (SELECT 1 FROM inserted i WHERE NOT EXISTS (SELECT 1 FROM deleted d WHERE d.ID = i.ID) )
        BEGIN
            RAISERROR ('Updates of ID and KindOf in LegalEntity are not allowed as they are referenced by RealPerson.', 16, 1);
            ROLLBACK TRANSACTION; -- optional, to recover the transaction
        END;
    END;
END;
GO

CREATE TRIGGER trgRealPersonCheck ON RealPerson
AFTER INSERT, UPDATE
AS
BEGIN
    IF EXISTS (SELECT 1 FROM inserted WHERE KindOf = 1 AND NOT EXISTS (SELECT 1 FROM NaturalPerson WHERE ID = inserted.ID))
    BEGIN
        RAISERROR ('For inserts / updates in RealPerson with KindOf = 1, an entry in NaturalPerson must exist.', 16, 1);
        ROLLBACK TRANSACTION;
        RETURN;
    END;

    IF EXISTS (SELECT 1 FROM inserted WHERE KindOf = 2 AND NOT EXISTS (SELECT 1 FROM LegalEntity WHERE ID = inserted.ID))
    BEGIN
        RAISERROR ('For inserts / updates in RealPerson with KindOf = 2, an entry in LegalEntity must exist.', 16, 1);
        ROLLBACK TRANSACTION;
        RETURN;
    END;
END;

GO

-- Test

INSERT INTO EntityOrPerson (ID, KindOf) VALUES (1, 1), (2, 1), (3, 1), (4, 2), (5, 1), (6, 2), (7, 2), (8, 1), (9, 2), (10, 1), (11, 1), (12, 1), (13, 2), (14, 1), (15, 2), (16, 2), (17, 1) 
GO

INSERT INTO NaturalPerson (ID, KindOf, Name, FirstName, Salutation, BirthDate) VALUES
       (1,  1, 'Fischer',   'Finn',      1, '1991-11-01'),
	   (2,  1, 'Herrmann',  'Charlotte', 2, '1980-12-17'),
	   (3,  1, 'Jung',      'Simon',     1, '1992-01-17'),
	   (5,  1, 'Weber',     'Emilia',    1, '1999-11-17'),
	   (8,  1, 'Huber',     'Manuel',    1, '1959-07-19'),
	   (10, 1, 'Meyer',     'Isa',       2, '1961-06-02'),
	   (11, 1, 'Gerlach',   'ELse',      2, '1955-11-23'),
	   (12, 1, 'Schreiner', 'Fred',      1, '1959-03-24'),
	   (14, 1, 'Friedrich', 'Anna',      2, '1972-08-18'),
	   (17, 1, 'Poe',       'Wilhelm',   1, '1968-10-26')

INSERT INTO RealPerson (ID, KindOf) VALUES (1, 1), (2, 1), (3, 1), (5, 1), (8, 1), (10, 1), (11, 1), (12, 1), (14, 1), (17, 1) 

INSERT INTO LegalEntity (ID, KindOf, Name, CorporateForm) VALUES
       ( 4, 2, 'Pfefferminzia AG', 2),
	   ( 6, 2, 'Inter Trade OHG', 4),
	   ( 7, 2, 'Testonia GmbH', 1),
	   ( 9, 2, 'Blabla GmbG', 1),
	   (13, 2, 'Trader for All', 1),
	   (15, 2, 'Compi Doktor', 3),
       (16, 2, 'Breakout GmbH', 1)


INSERT INTO RealPerson (ID, KindOf) VALUES (4, 2), (6, 2), (7, 2), (9, 2), (13, 2), (15, 2), (16, 2)

/*
INSERT INTO EntityOrPerson (ID, KindOf) VALUES (10, 3)

INSERT INTO NaturalPerson (ID, KindOf, Name, FirstName, BirthDay) VALUES
       (4, 2, 'Mayer', 'Sandra', '1974-12-31')

INSERT INTO NaturalPerson (ID, KindOf, Name, FirstName, BirthDay) VALUES
       (10, 1, 'Müller', 'Carolin', '1984-03-02')

DELETE FROM NaturalPerson WHERE ID = 5

DELETE FROM LegalEntity WHERE ID = 7

INSERT INTO EntityOrPerson (ID, KindOf) VALUES (20, 1), (21,2)
INSERT INTO RealPerson (ID, KindOf) VALUES (20, 1)
INSERT INTO RealPerson (ID, KindOf) VALUES (21,2)

INSERT INTO EntityOrPerson (ID, KindOf) VALUES (20, 1), (21,2)
INSERT INTO RealPerson (ID, KindOf) VALUES (20, 1)
INSERT INTO RealPerson (ID, KindOf) VALUES (21,2)

INSERT INTO EntityOrPerson (ID, KindOf) VALUES (30, 1)
INSERT INTO RealPerson (ID, KindOf) VALUES (30, 1)

INSERT EntityOrPerson (ID, KindOf) VALUES  (22, 1)
UPDATE RealPerson SET ID = 22, KindOf = 1 WHERE ID = 9

*/



