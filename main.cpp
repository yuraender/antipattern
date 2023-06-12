/*
Модуль текстового редактора.
 
 Возмоности текстового редактора.
 
 Создавать и редактировать документы, которые состоят из формитированного текста и фигур.
 Текст и фигуры могут располагаться в виде блоков, следующих в произвольном порядке, обтекание фигур не
 предусмотрено техническим заданием.
 
 Текстовый блок состоит из фрагментов текста. Фрагмент текста несёт содержание и опционально стиль, отличающийся от стиля текстового блока.
 Текстоыый блок не может быть произвольным образом масштабирован.
 
 Фигура представляет собой набор геометрических примитивов. Для неё задаётся единый стиль и её размер может быть изменён определённым образом.
 Для работы с фигурами требуется библиотека, имеющая заранее заданный интерфейс и требующая активации при подключении к приложению.
 
 В документе возможен поиск всех элементов текста, соответствующих шаблону с возможностью уточнять стиль.
 
 Документ может быт отправлен для отображения на экране без разбиения на страницы или в PDF с разбиением. На следующем этапе разработки планируется вывод
 адаптивного представления (десктоп/планшет/телефон/web.)
 

 */

#include <utility>
#include <string>
#include <list>
#include <memory>
#include <array>

using namespace std;

class IWindowManager;
class IODTWriter;
class IPDFWriter;
class IMobileDrawer;
class ITabletDisplay;


class ISpellChecker;


class GraphEngineManager // Взят из библиоткеи двумерных примитивов
{
public:
  virtual void Activate( const char* key ) = 0;
  virtual bool ActivateState() const = 0;
  virtual int  LastErrorCode() = 0;
};

class IPlaneItem; // Определён в библиотеке двумерных примитивов

class TextStyle
{
  string styleName;
  unsigned int color, background;
  string font;
  unsigned int fontSize;
  bool bold, italic, underlined, crossed;
public:
  TextStyle( const string& name, unsigned int col, unsigned int backgr, string& fnt, const array<bool,4>& textDecor )
  : styleName{ name }, color(col), background(backgr), font(fnt), bold(textDecor[0]), italic(textDecor[1]), underlined(textDecor[2]), crossed(textDecor[3])
  {}
  TextStyle() = default;
  TextStyle( const TextStyle& ) = default;
};


class ITextEditorItem
{
public:
  virtual bool CheckSpelling( const ISpellChecker& ) = 0;
  virtual pair<int, int> GetGabarit() const { return pair<int,int>(-1,-1); }
  virtual void SetFont( const string& , int fontSize, bool italic, bool bold, long int displayMask ) const = 0;
  virtual void SetLineWidth( long int width ) const = 0;
  
  
  virtual bool SetGabarit( const std::pair<int, int>& ) const = 0;
  
  virtual void SaveFile( IODTWriter* ) = 0;
  virtual void ToWindow( IWindowManager* ) = 0;
  virtual bool ToPDF( IPDFWriter& ) { return 0; }
  //Зарезервировано на будущую разработку
  //virtual bool ToMobile( IMobileDrawer ) = 0;
  //virtual bool ToTablet( ITabletDisplay ) = 0;
};


class FormattedText : public ITextEditorItem
{
  string text;
  unsigned int color, background;
  string font;
  unsigned int fontSize;
  bool bold, italic, underlined, crossed;
  shared_ptr<TextStyle> sytleInheritFrom;
  
public:
  bool CheckSpelling( const ISpellChecker& ) override; // implemented in cpp

  std::pair<int, int> GetGabarit() const  override; // implemented in cpp
  void SetFont( const std::string& , int fontSize, bool italic, bool bold, long int displayMask ) const  override; // implemented in cpp
  void SetLineWidth( long int width ) const  override {/*nothing to implement*/};
  
  
  bool SetGabarit( const std::pair<int, int>& ) const  override {/*nothing to implement*/ return false;};
  
  void SaveFile( IODTWriter* ) override; // // implemented in cpp
  void ToWindow( IWindowManager* ) override; // implemented in cpp
  bool ToPDF( IPDFWriter& ) override; // implemented in cpp, returns true
  
  string Text() const { return text; }
  
  TextStyle GetStyle()
  {
    //TextStyle result{ string{}, color, background, font, array<bool,4>{ bold, italic, underlined, crossed } };
    array<bool,4> textMod{ bold, italic, underlined, crossed };
    TextStyle st ( string{}, color, background, font, textMod );
    return sytleInheritFrom ? *sytleInheritFrom.get() : st;
  }
};


class Paragraph : public ITextEditorItem
{
  vector<shared_ptr<FormattedText>> textItems;
public:
  size_t CountItems() const { return textItems.size(); }
  shared_ptr<ITextEditorItem> TextItem( size_t index ) { return textItems.at( index ); }
};


class Figure : public ITextEditorItem
{
  unsigned int color, background;
  vector<shared_ptr<IPlaneItem>> shapeItems;
  GraphEngineManager* geometricEngine;
  
public:
  
  Figure( int col, int back, GraphEngineManager& geomToolkit, const char* keyToActivate )
  : color(col), background(back), shapeItems(), geometricEngine(&geomToolkit)
  {
    if ( !geometricEngine->ActivateState() )
      geometricEngine->Activate(keyToActivate);
  }
  
  bool CheckSpelling( const ISpellChecker& )  override { return false; }

  std::pair<int, int> GetGabarit() const  override; // implemented in cpp
  void SetFont( const std::string& , int fontSize, bool italic, bool bold, long int displayMask ) const  override {/*не требуется реализация*/}
  void SetLineWidth( long int width ) const  override; // implememnted in cpp
  
  
  bool SetGabarit( const std::pair<int, int>& ) const  override{ /* CalculateGabarit; Scale*/ return true;};
  
  void SaveFile( IODTWriter* ) override; // // implemented in cpp
  void ToWindow( IWindowManager* ) override; // implemented in cpp
  bool ToPDF( IPDFWriter& ) override; // implemented in cpp, returns true
};


class TextDocument
{
  vector<shared_ptr<ITextEditorItem>> textItems;
public:
  bool InsertItem( ITextEditorItem& itemToAdd, const ITextEditorItem* insertAfter )
  {
    if ( dynamic_cast<FormattedText*>(&itemToAdd) )
      return false;
    else
      return true; // И реально вставляем элемент
  }
  size_t CountItems() const { return textItems.size(); }
  shared_ptr<ITextEditorItem> TextItem( size_t index ) { return textItems.at( index ); }
};


struct ITextPatternFinder
{
  virtual bool operator()( const string& ) const = 0;
};


struct IStylePatternFinder
{
  virtual bool operator()( const TextStyle& ) const = 0;
};



vector<shared_ptr<FormattedText>> FindTextByPattern( TextDocument& doc, ITextPatternFinder& pattern, IStylePatternFinder* stylePattern )
{
  vector<shared_ptr<FormattedText>> result;
  for( size_t iBlock = 0; iBlock < doc.CountItems(); iBlock++ )
  {
    auto block = doc.TextItem(iBlock);
    if ( auto asParagrgaph = dynamic_cast<Paragraph*>(block.get()) )
    {
      for( auto iText = 0; iText < asParagrgaph->CountItems(); iText++ )
      {
        if ( auto asText = dynamic_cast<FormattedText*>(asParagrgaph->TextItem( iText ).get()) )
        {
          if ( pattern( asText->Text() ) )
          {
            if ( stylePattern )
            {
              if ( (*stylePattern)(asText->GetStyle() ) )
                result.push_back( shared_ptr<FormattedText>(asText) );
            }
            else
              result.push_back( shared_ptr<FormattedText>(asText) );
          }
        }
      }
    }
  }
  return result;
}



int main()
{
  
  
  return 0;
}
