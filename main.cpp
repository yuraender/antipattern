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

struct TextStyle {
  string styleName;
  unsigned int color, background;
  string font;
  unsigned int fontSize;
  bool bold, italic, underline, strikethrough;

  TextStyle(const string& name, unsigned int col, unsigned int backgr, string& fnt, unsigned int fntSize,
            const array<bool, 4>& textDecor
  ): styleName{name}, color(col), background(backgr), font(fnt), fontSize(fntSize), bold(textDecor[0]),
     italic(textDecor[1]), underline(textDecor[2]), strikethrough(textDecor[3]) {}

  TextStyle(const TextStyle&) = default;
};

class Text;

class FormattedText;

class Paragraph;

class Figure;

class ITextEditorItemVisitor {
  public:
  virtual void Visit(Text* item) = 0;

  virtual void Visit(FormattedText* item) = 0;

  virtual void Visit(Paragraph* item) = 0;

  virtual void Visit(Figure* item) = 0;
};

class ITextEditorItem {
  public:
  virtual void SetLineWidth(long int width) const { /* nothing to implement */ }

  virtual long int GetLineWidth() const { return -1; }

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
  void Visit(Text* item) override {}

  void Visit(FormattedText* item) override {}

  void Visit(Paragraph* item) override {}

  void Visit(Figure* item) override {}
};

class Text : public ITextEditorItem {
  public:
  virtual bool CheckSpelling(const ISpellChecker&) { return false; } // implemented in cpp

  void SetLineWidth(long int width) const override { /* nothing to implement */ }

  long int GetLineWidth() const override { return -1; }

  void SaveFile(IODTWriter*) override {} // implemented in cpp

  void ToWindow(IWindowManager*) override {} // implemented in cpp

  bool ToPDF(IPDFWriter&) override { return true; } // implemented in cpp, returns true

  virtual string GetText() const {
    return text;
  }

  void Accept(ITextEditorItemVisitor* visitor) override {
    visitor->Visit(this);
  }

  protected:
  string text;
};

class FormattedText : public Text {
  public:
  FormattedText() {}

  virtual void SetStyle(TextStyle& style) {
    this->style = &style;
  }

  virtual TextStyle GetStyle() {
    array<bool, 4> textMod{style->bold, style->italic, style->underline, style->strikethrough};
    TextStyle st(style->styleName, style->color, style->background, style->font, style->fontSize, textMod);
    return styleInheritFrom ? *styleInheritFrom : st;
  }

  void Accept(ITextEditorItemVisitor* visitor) override {
    visitor->Visit(this);
  }

  private:
  TextStyle* style;
  shared_ptr<TextStyle> styleInheritFrom;
};

class FormattedTextDecorator : public FormattedText {
  public:
  FormattedTextDecorator(std::shared_ptr<FormattedText> text)
      : text(text) {}

  virtual TextStyle GetStyle() const {
    return text->GetStyle();
  }

  protected:
  std::shared_ptr<FormattedText> text;
};

class BoldText : public FormattedTextDecorator {
  public:
  BoldText(std::shared_ptr<FormattedText> text)
      : FormattedTextDecorator(text) {}

  TextStyle GetStyle() const override {
    TextStyle style = text->GetStyle();
    style.bold = true;
    return style;
  }
};

class ItalicText : public FormattedTextDecorator {
  public:
  ItalicText(std::shared_ptr<FormattedText> text)
      : FormattedTextDecorator(text) {}

  TextStyle GetStyle() const override {
    TextStyle style = text->GetStyle();
    style.italic = true;
    return style;
  }
};

class UnderlineText : public FormattedTextDecorator {
  public:
  UnderlineText(std::shared_ptr<FormattedText> text)
      : FormattedTextDecorator(text) {}

  TextStyle GetStyle() const override {
    TextStyle style = text->GetStyle();
    style.underline = true;
    return style;
  }
};

class StrikethroughText : public FormattedTextDecorator {
  public:
  StrikethroughText(std::shared_ptr<FormattedText> text)
      : FormattedTextDecorator(text) {}

  TextStyle GetStyle() const override {
    TextStyle style = text->GetStyle();
    style.strikethrough = true;
    return style;
  }
};

class Paragraph : public ITextEditorItem {
  public:
  void Accept(ITextEditorItemVisitor* visitor) override {
    visitor->Visit(this);
  }

  VectorIterator<shared_ptr<Text>> iterator() {
    return VectorIterator<shared_ptr<Text>>{textItems};
  }

  private:
  vector<shared_ptr<Text>> textItems;
};

class Figure : public ITextEditorItem {
  public:
  Figure(int col, int back, const char* keyToActivate)
      : color(col), background(back), shapeItems() {
    if (!GraphEngineManager::GetInstance().ActivateState())
      GraphEngineManager::GetInstance().Activate(keyToActivate);
  }

  virtual pair<int, int> GetGabarit() const { return std::pair{-1, -1}; } // implemented in cpp

  bool SetGabarit(const std::pair<int, int>&) const { /* CalculateGabarit; Scale */ return true; }

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
    if (dynamic_cast<Text*>(&itemToAdd))
      return false;
    else {
      std::shared_ptr<ITextEditorItem> sptr(&itemToAdd);
      textItems.push_back(sptr);
      return true; // И реально вставляем элемент
    }
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

vector<shared_ptr<Text>> FindTextByPattern(
    TextDocument& doc, ITextPatternFinder& pattern, IStylePatternFinder* stylePattern
) {
  vector<shared_ptr<Text>> result;
  for (auto it = doc.iterator(); *it != nullptr; ++it) {
    auto block = *it;
    if (auto asParagraph = dynamic_cast<Paragraph*>(block.get())) {
      for (auto itText = asParagraph->iterator(); *itText != nullptr; ++itText) {
        auto text = *itText;
        if (auto asText = dynamic_cast<Text*>(text.get())) {
          if (pattern(asText->GetText())) {
            if (stylePattern) {
              if (auto asFormattedText = dynamic_cast<FormattedText*>(text.get())) {
                if ((*stylePattern)(asFormattedText->GetStyle()))
                  result.push_back(shared_ptr<FormattedText>(asFormattedText));
              }
            } else
              result.push_back(shared_ptr<Text>(asText));
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
        if (auto asText = dynamic_cast<Text*>(text.get())) {
          if (!sc(asText->GetText())) {
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
