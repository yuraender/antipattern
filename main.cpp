/*
 Модуль текстового редактора.

 Возможности текстового редактора.

 Создавать и редактировать документы, которые состоят из форматированного текста и фигур.
 Текст и фигуры могут располагаться в виде блоков, следующих в произвольном порядке, обтекание фигур не
 предусмотрено техническим заданием.
 
 Текстовый блок состоит из фрагментов текста. Фрагмент текста несёт содержание и опционально стиль,
 отличающийся от стиля текстового блока.
 Текстовый блок не может быть произвольным образом масштабирован.
 
 Фигура представляет собой набор геометрических примитивов. Для неё задаётся единый стиль, и её размер может быть
 изменён определённым образом.
 Для работы с фигурами требуется библиотека, имеющая заранее заданный интерфейс и требующая активации
 при подключении к приложению.
 
 В документе возможен поиск всех элементов текста, соответствующих шаблону с возможностью уточнять стиль.
 
 Документ может быт отправлен для отображения на экране без разбиения на страницы или в PDF с разбиением.
 На следующем этапе разработки планируется вывод адаптивного представления (десктоп/планшет/телефон/web).
 */

#include <array>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "patterns/iterator.h"

using namespace std;

class IWindowManager;

class IODTWriter;

class IPDFWriter;

class IMobileDrawer;

class ITabletDisplay;

class ISpellChecker {
  public:
  virtual bool operator()(const string&) const = 0;
};

class IGraphEngineManager // Взят из библиотеки двумерных примитивов
{
  public:
  virtual void Activate(const char* key) = 0;

  virtual bool ActivateState() const = 0;

  virtual int LastErrorCode() = 0;
};

class GraphEngineManager : IGraphEngineManager {
  public:
  GraphEngineManager(const GraphEngineManager&) = delete;

  GraphEngineManager& operator=(const GraphEngineManager&) = delete;

  void Activate(const char* key) override {}

  bool ActivateState() const override { return false; }

  int LastErrorCode() override { return 0; }

  static GraphEngineManager& GetInstance() {
    if (instance == nullptr) {
      std::lock_guard<std::mutex> lock(mutex);
      if (instance == nullptr) {
        instance = std::unique_ptr<GraphEngineManager>(new GraphEngineManager());
      }
    }
    return *instance;
  }

  private:
  GraphEngineManager() {}

  static std::unique_ptr<GraphEngineManager> instance;
  static std::mutex mutex;
};

std::unique_ptr<GraphEngineManager> GraphEngineManager::instance = nullptr;
std::mutex GraphEngineManager::mutex;

class IPlaneItem; // Определён в библиотеке двумерных примитивов

class TextStyle {
  private:
  string styleName;
  unsigned int color, background;
  string font;
  unsigned int fontSize;
  bool bold, italic, underlined, crossed;

  public:
  TextStyle(const string& name, unsigned int col, unsigned int backgr, string& fnt, unsigned int fntSize,
            const array<bool, 4>& textDecor
  ): styleName{name}, color(col), background(backgr), font(fnt), fontSize(fntSize), bold(textDecor[0]),
     italic(textDecor[1]), underlined(textDecor[2]), crossed(textDecor[3]) {}

  TextStyle(const TextStyle&) = default;
};

class FormattedText;

class Paragraph;

class Figure;

class ITextEditorItemVisitor {
  public:
  virtual void Visit(FormattedText* item) = 0;

  virtual void Visit(Paragraph* item) = 0;

  virtual void Visit(Figure* item) = 0;
};

class ITextEditorItem {
  public:
  virtual bool CheckSpelling(const ISpellChecker&) = 0;

  virtual pair<int, int> GetGabarit() const { return pair<int, int>(-1, -1); }

  virtual void SetFont(const string&, int fontSize, bool italic, bool bold, long int displayMask) const = 0;

  virtual void SetLineWidth(long int width) const = 0;

  virtual long int GetLineWidth() const = 0;

  virtual bool SetGabarit(const std::pair<int, int>&) const = 0;

  virtual void SaveFile(IODTWriter*) = 0;

  virtual void ToWindow(IWindowManager*) = 0;

  virtual bool ToPDF(IPDFWriter&) = 0;

  //Зарезервировано на будущую разработку
  //virtual bool ToMobile( IMobileDrawer ) = 0;
  //virtual bool ToTablet( ITabletDisplay ) = 0;

  virtual void Accept(ITextEditorItemVisitor* visitor) = 0;
};

class TextEditorItemVisitor : public ITextEditorItemVisitor {
  public:
  void Visit(FormattedText* item) override {}

  void Visit(Paragraph* item) override {}

  void Visit(Figure* item) override {}
};

class FormattedText : public ITextEditorItem {
  public:
  bool CheckSpelling(const ISpellChecker&) override { return false; } // implemented in cpp

  std::pair<int, int> GetGabarit() const override { return pair<int, int>(-1, -1); } // implemented in cpp

  void SetFont(
      const std::string&, int fontSize, bool italic, bool bold, long int displayMask
  ) const override {} // implemented in cpp

  void SetLineWidth(long int width) const override { /* nothing to implement */ }

  long int GetLineWidth() const override { return -1; }

  bool SetGabarit(const std::pair<int, int>&) const override { /*nothing to implement */ return false; }

  void SaveFile(IODTWriter*) override {} // implemented in cpp

  void ToWindow(IWindowManager*) override {} // implemented in cpp

  bool ToPDF(IPDFWriter&) override { return true; } // implemented in cpp, returns true

  string Text() const { return text; }

  TextStyle GetStyle() {
    array<bool, 4> textMod{bold, italic, underlined, crossed};
    //TextStyle result{ string{}, color, background, font, array<bool,4>{ bold, italic, underlined, crossed } };
    TextStyle st(string{}, color, background, font, fontSize, textMod);
    return styleInheritFrom ? *styleInheritFrom : st;
  }

  void Accept(ITextEditorItemVisitor* visitor) override {
    visitor->Visit(this);
  }

  private:
  string text;
  unsigned int color, background;
  string font;
  unsigned int fontSize;
  bool bold, italic, underlined, crossed;
  shared_ptr<TextStyle> styleInheritFrom;
};

class Paragraph : public ITextEditorItem {
  public:
  void Accept(ITextEditorItemVisitor* visitor) override {
    visitor->Visit(this);
  }

  VectorIterator<shared_ptr<FormattedText>> iterator() {
    return VectorIterator<shared_ptr<FormattedText>>{textItems};
  }

  private:
  vector<shared_ptr<FormattedText>> textItems;
};

class Figure : public ITextEditorItem {
  public:
  Figure(int col, int back, const char* keyToActivate)
      : color(col), background(back), shapeItems() {
    if (!GraphEngineManager::GetInstance().ActivateState())
      GraphEngineManager::GetInstance().Activate(keyToActivate);
  }

  bool CheckSpelling(const ISpellChecker&) override { return false; }

  std::pair<int, int> GetGabarit() const override { return std::pair{-1, -1}; } // implemented in cpp

  void SetFont(const std::string&, int fontSize, bool italic, bool bold,
               long int displayMask) const override { /* не требуется реализация */ }

  void SetLineWidth(long int width) const override {} // implemented in cpp

  long int GetLineWidth() const override { return -1; } // implemented in cpp

  bool SetGabarit(const std::pair<int, int>&) const override { /* CalculateGabarit; Scale */ return true; }

  void SaveFile(IODTWriter*) override {} // implemented in cpp

  void ToWindow(IWindowManager*) override {} // implemented in cpp

  bool ToPDF(IPDFWriter&) override { return true; } // implemented in cpp, returns true

  void Accept(ITextEditorItemVisitor* visitor) override {
    visitor->Visit(this);
  }

  VectorIterator<shared_ptr<IPlaneItem>> iterator() {
    return VectorIterator<shared_ptr<IPlaneItem>>{shapeItems};
  }

  private:
  unsigned int color, background;
  vector<shared_ptr<IPlaneItem>> shapeItems;
};

class TextDocument {
  public:
  bool InsertItem(ITextEditorItem& itemToAdd, const ITextEditorItem* insertAfter) {
    if (dynamic_cast<FormattedText*>(&itemToAdd))
      return false;
    else
      return true; // И реально вставляем элемент
  }

  VectorIterator<shared_ptr<ITextEditorItem>> iterator() {
    return VectorIterator<shared_ptr<ITextEditorItem>>{textItems};
  }

  private:
  vector<shared_ptr<ITextEditorItem>> textItems;
};

struct ITextPatternFinder {
  virtual bool operator()(const string&) const = 0;
};

struct IStylePatternFinder {
  virtual bool operator()(const TextStyle&) const = 0;
};

vector<shared_ptr<FormattedText>> FindTextByPattern(
    TextDocument& doc, ITextPatternFinder& pattern, IStylePatternFinder* stylePattern
) {
  vector<shared_ptr<FormattedText>> result;
  for (auto it = doc.iterator(); *it != nullptr; ++it) {
    auto block = *it;
    if (auto asParagraph = dynamic_cast<Paragraph*>(block.get())) {
      for (auto itText = asParagraph->iterator(); *itText != nullptr; ++itText) {
        auto text = *itText;
        if (auto asText = dynamic_cast<FormattedText*>(text.get())) {
          if (pattern(asText->Text())) {
            if (stylePattern) {
              if ((*stylePattern)(asText->GetStyle()))
                result.push_back(shared_ptr<FormattedText>(asText));
            } else
              result.push_back(shared_ptr<FormattedText>(asText));
          }
        }
      }
    }
  }
  return result;
}

void SpellCheck(TextDocument& doc, ISpellChecker& sc) {
  for (auto it = doc.iterator(); *it != nullptr; ++it) {
    auto block = *it;
    if (auto asParagraph = dynamic_cast<Paragraph*>(block.get())) {
      for (auto itText = asParagraph->iterator(); *itText != nullptr; ++itText) {
        auto text = *itText;
        if (auto asText = dynamic_cast<FormattedText*>(text.get())) {
          if (!sc(asText->Text())) {
            // inform user
          }
        }
      }
    }
  }
}

void SetLineWidthGreaterThan(TextDocument& doc, long int minimalWidth) {
  for (auto it = doc.iterator(); *it != nullptr; ++it) {
    auto block = *it;
    if (auto asFigure = dynamic_cast<Figure*>(block.get())) {
      if (asFigure->GetLineWidth() < minimalWidth)
        asFigure->SetLineWidth(minimalWidth);
    }
  }
}

int main() {
  return 0;
}
