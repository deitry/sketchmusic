#pragma once
#include "pch.h"
#include "math.h"
#include "ppltasks.h"
#include "agents.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace concurrency;

#define SOURCE_VOICES_MAX		16		// максимальное количество голосов
#define DISCRETIZATION_FREQ		44100	// частота дискретизации
#define MAX_TEXT_LENGTH			255		// максимальная длина одной строки текста ~ 64 такта 4/4. Хочешь больше - переходи на новую строку

/**
 * Пространство имён, объединяющее базовые классы для построения музыкального редактора.
 */
namespace SketchMusic
{
	const int TICK_IN_BEAT = 32;	// количество тиков в доле
	const int BPM = 120;
	const int MAX_COMMAND_COUNT = 20; // максимальное число команд в истории
	const int MIDIKEYTOA4 = 69;

	ref class Text;
	ref class TextIterator;
	ref class Cursor;
	ref class PartDefinition;
	ref class Idea;
	ref class CompositionHeader;
	ref class CompositionData;
	//ref class CompositionLibrary;
	ref class Composition;
	ref class PositionedIdea;
	
	public enum class IdeaCategoryEnum
		// категория, к которой относится идея. Будет использоваться 
		// для выбора отображения редактора идеи и для сортировки. Может ещё где-нибудь
		// TODO : отдельные категории (?) : functional (тоники, доминанты)
		// transitions (переходы - специальный подвид мелодий), loops 
	{
		none = 0,		// по умолчанию будем считать, что идея не принадлежит никакой категории.
			// если что, обрабатывать будем аналогично "комбинированной" идее ...
		name = 1,		// идея определяет по сути только название
		instrument = 2,	// идея определяет используемый инструмент (как вариант - способ звукоизвлечения)
		tempo = 6,		// задаёт темп и размер. Можно использовать как базу для создания композиции,
						// когда вообще не знаешь о чём думать.

		rhythm = 10,			// задаёт некоторый ритм
		melody = 11,			// мелодическая последовательность + ритм
		chord = 13,				// один аккорд
		melodicSequence = 15,	// мелодическая последовательность - небор нот без привязки к ритму
		chordProgression = 16,	// гармоническая последовательность - набор аккордов без привязки к ритму?

		shape = 20,		// форма композиции. Такая категория идей обязательно должна быть, осталось понять,
		// как это можно увязать с представлением идеи в качестве текста - с помощью специальных "символов формы"?
		// "Символы формы" могут указывать, где начинается одна часть и заканчивается другая.
		// Символы "перехода на новую строку" могут быть также частью символов формы
		// TODO : нужен символ начала новой части / подчасти - два отдельных вида символов? 
		// Разумнее сделать один тип с указанием "родительского объекта" - тогда можно будет представить все символы в виде иерархии/дерева
		// TODO : к каждой отдельной части скорее всего пригодится прикладывать описание или ещё какую-нибудь идею
		// TODO : длительности частей?

		dynamic = 30,	// указание динамики/текстуры (нутыпонел). Как ритм и акцентирование вместе,
		// только в более общем виде, без конкретизации

		lyrics = 90,	// задаёт слова - что не влезает в название
		combo = 100		// идея определяет сразу кучу параметров
	};

	// Категория динамики. Сделал через битовые поля, чтобы одним числом можно было определить сразу несколько категорий.
	public enum class DynamicCategory
	{
		quite		= 0,	// |--
		regular		= 1,	// ||
		unregular	= 2, 	// |/|
		fast		= 3,	// ||||
		hard		= 4,	// !
		harder		= 5,	// !||
		solo		= 6,	// ~
	};

	ref class Sample;
	ref class Instrument;
	ref class InstrumentToTextConverter;

	

	// внутренняя структура

	// список используемых типов символов
	public enum class SymbolType
	{
		unknown	= 0,	// тип не определён или тип не важен (если используется в качестве аргумента)
		
		NOTE	= 1,	// обычная нота
		RNOTE	= 2,	// относительная нота (относительно последней "обычной")
		GNOTE	= 3,	// обобщённая нота (не имеет непосредственной связи с высотой звука)
		END		= 4,	// прекращение звучания нот(-ы)
		CHORD	= 5,	// ранее известен как SCALE - определяет аккорд (в обобщённом, неконкретизированном виде) / гармонию / звукоряд

		NLINE	= 6,	// новая строка - для визуального отображения
		SPACE	= 7,	// "пробел" - для визуального отображения
		NPART	= 8,	// новый раздел - для отделения частей одна от другой

		TEMPO	= 9,	// задание темпа
		STRING	= 10,	// для вставки строки		
		CLEF	= 11,	// для установки нот по вертикали
		ACCENT	= 12,	// для "подсветки" данного положения - таким образом будем выделять ключевые места.
						// По сути, ту же самую роль может выполнять GNote, но этот элемент будет проще, безо всяких значений,
						// а также будет служить для разового "смещения" метронома
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class IdeaCategoryToStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
		{
			if (value == nullptr) return nullptr;
			IdeaCategoryEnum type = (IdeaCategoryEnum)(value);
			//int res = static_cast<int>(type);
			switch (type)
			{
			case IdeaCategoryEnum::chord: return "Аккорд";
			case IdeaCategoryEnum::chordProgression: return "Последовательность аккордов";
			case IdeaCategoryEnum::combo: return "Комбинированная идея";
			case IdeaCategoryEnum::dynamic: return "Динамика";
			case IdeaCategoryEnum::instrument: return "Инструмент";
			case IdeaCategoryEnum::lyrics: return L"\uE8E4";
			case IdeaCategoryEnum::melodicSequence: return L"\ue189";
			case IdeaCategoryEnum::melody: return L"\ue189";
			case IdeaCategoryEnum::name: return "Название";
			case IdeaCategoryEnum::rhythm: return "Ритм";
			case IdeaCategoryEnum::shape: return "Форма";
			case IdeaCategoryEnum::tempo: return "Темп";
			case IdeaCategoryEnum::none:
			default: return "Неизвестный тип";
			}
			return "";
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			if (value == nullptr) return nullptr;
			int res = std::wcstol(dynamic_cast<String^>(value)->Data(), NULL, 10);
			return res;
		}

		IdeaCategoryToStrConverter() {}
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class VerboseIdeaCategoryToStrConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
		{
			if (value == nullptr) return nullptr;
			IdeaCategoryEnum type = (IdeaCategoryEnum)(value);
			//int res = static_cast<int>(type);
			switch (type)
			{
			case IdeaCategoryEnum::chord: return "Аккорд";
			case IdeaCategoryEnum::chordProgression: return "Последовательность аккордов";
			case IdeaCategoryEnum::combo: return "Комбинированная идея";
			case IdeaCategoryEnum::dynamic: return "Динамика";
			case IdeaCategoryEnum::instrument: return "Инструмент";
			case IdeaCategoryEnum::lyrics: return "Текст/стихи";
			case IdeaCategoryEnum::melodicSequence: return "Мелодическая последовательность";
			case IdeaCategoryEnum::melody: return "Мелодия";
			case IdeaCategoryEnum::name: return "Название";
			case IdeaCategoryEnum::rhythm: return "Ритм";
			case IdeaCategoryEnum::shape: return "Форма";
			case IdeaCategoryEnum::tempo: return "Темп";
			case IdeaCategoryEnum::none:
			default: return "Неизвестный тип";
			}
			return "";
		}
		virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
		{
			if (value == nullptr) return nullptr;
			int res = std::wcstol(dynamic_cast<String^>(value)->Data(), NULL, 10);
			return res;
		}

		VerboseIdeaCategoryToStrConverter() {}
	};

	ref class TimeToStrConverter;

	public interface class ISymbol
	{
	public:
		SymbolType GetSymType();
		Platform::String^ ToString();	// представление в текстовом виде значения
		
		bool EQ(ISymbol^ second);
	};

	public interface class INote
	{
	public:
		property int _val;
		property int _velocity;
		property int _voice;
		//property Platform::String^ str; // ? для будущего, когда появятся слова и подписи
	};

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class PositionedSymbol sealed
	{
	public:
		property SketchMusic::Cursor^ _pos;
		property SketchMusic::ISymbol^ _sym;

		PositionedSymbol() { }
		PositionedSymbol(Cursor^ pos, ISymbol^ sym) { _pos = pos; _sym = sym; }
	};

	[Windows::UI::Xaml::Data::Bindable]
	public ref class SNote sealed : public SketchMusic::ISymbol, public SketchMusic::INote
	{
	private:
		static Platform::String^ valToString(int val);	// то ли перенеси функцию в INote,
			// то ли вообще вынести в отдельный класс, который занимается преобразованиями
	public:
		SNote() { _val = 0; }
		SNote(int tone) { _val = tone; _velocity = 0; _voice = 0; }
		SNote(int tone, int velocity, int voice) { _val = tone; _velocity = velocity; _voice = voice; }

		virtual property int _val;
		virtual property int _velocity;
		virtual property int _voice;

		virtual SymbolType GetSymType() { return SymbolType::NOTE; }
		virtual Platform::String^ ToString() { 
			String^ str;
			switch (_val % 12)
			{
			case 0: str += "A"; break;
			case 1: case -11: str += "A#"; break;
			case 2: case -10: str += "B"; break;
			case 3: case -9: str += "C"; break;
			case 4: case -8: str += "C#"; break;
			case 5: case -7: str += "D"; break;
			case 6: case -6: str += "D#"; break;
			case 7: case -5: str += "E"; break;
			case 8: case -4: str += "F"; break;
			case 9: case -3: str += "F#"; break;
			case 10: case -2: str += "G"; break;
			case 11: case -1: str += "G#"; break;
			default: break;
			}
			str += ((_val + MIDIKEYTOA4 - 9) / 12 - 1);
			return str;
		}

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol^ second);
		//virtual Platform::String^ Serialize() { return "note," + valToString(_val); }	// приставка указывает на тип
	};

	public ref class SSpace sealed : public SketchMusic::ISymbol
	{
	public:
		SSpace() {}

		virtual SymbolType GetSymType() { return SymbolType::SPACE; }
		virtual Platform::String^ ToString() { return L"\uE75D"; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "space"; }
	};

	// акцент - как визуальный, так и звуковой.
	// Для него будет реализовано специальное поведение проигрывателя - если встречается этот символ,
	// то вне зависимости от текущего "тика", мы перескакиваем на ближайшую долю - принудительно делаем ноту "под ударением"
	// Соответственно будет играть метроном. Эти символы будут надстройкой над размером; при 
	public ref class SAccent sealed : public SketchMusic::ISymbol
	{
	public:
		SAccent() {}

		virtual SymbolType GetSymType() { return SymbolType::ACCENT; }
		virtual Platform::String^ ToString() { return L"\uE9A9"; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "space"; }
	};


	const int TrebleClef	= -5;	// скрипичный ключ; нижняя строка = ми4
	const int BassClef		= -14;	// басовый ключ; нижняя строка = соль3
	//const int AltoClef		= 0;
	//const int TenorClef		= 0;
	//const int BaritoneClef	= 0;


	public ref class SClef sealed : public SketchMusic::ISymbol
	{
	public:
		SClef() {}
		SClef(int val) { bottom = val; }

		property int bottom; // устанавливает высоту звука для нижней линии на нотном стане.
		
		virtual SymbolType GetSymType() { return SymbolType::CLEF; }
		virtual Platform::String^ ToString() { return "Clef"; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
	};

	public ref class SNewPart sealed : public SketchMusic::ISymbol
	{
	public:
		SNewPart() {}
		SNewPart(String^ _cat) { category = _cat; }
		SNewPart(String^ _name, String^ _cat, DynamicCategory _dyn) { Name = _name; category = _cat; dynamic = _dyn; }

		property String^ Name;
		property String^ category;	// "A", "B", "куплет", "бридж" - определяется только категория
			//, номер будет отображаться автоматически в зависимости от положения в композиции
		
		property DynamicCategory dynamic;

		virtual SymbolType GetSymType() { return SymbolType::NPART; }
		virtual Platform::String^ ToString() { return L"\uE70B" + category; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
	};

	// Вместо использования PositionedSymbol для оперирования на уровне частей в редакторе композиции,
	// будем создавать специальный объект, для этого предназначенный
	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class PartDefinition sealed : Windows::UI::Xaml::Data::INotifyPropertyChanged
	{
	private:
		void OnPropertyChanged(Platform::String^ propertyName);
		int m_number;
		int m_length;
		double m_time;
	public:
		PartDefinition() {}
		PartDefinition(PositionedSymbol^ psym) { originalPos = psym->_pos; original = dynamic_cast<SNewPart^>(psym->_sym); }
		PartDefinition(Cursor^ pos, SNewPart^ sym) { originalPos = pos; original = sym; }

		PositionedSymbol^ GetPositionedSymbol(Cursor^ cur) 
		{ 
			if (cur) return ref new PositionedSymbol(cur, original); 
			else return ref new PositionedSymbol(originalPos, original);
		}

		property Cursor^ originalPos;
		property SNewPart^ original;

		property int Number		// порядковый номер части с этой категорией. Будет вычисляться автоматически, здесь храним для удобства
		{
			int get() { return m_number; }
			void set(int num) { m_number = num; OnPropertyChanged("Number"); }
		}
		property int Length	// продолжительность в "битах". Будет вычисляться автоматически, здесь храним для удобства
		{
			int get() { return m_length; }
			void set(int num) { m_length = num; OnPropertyChanged("Length"); }
		}
		property double Time
		{
			double get() { return m_time; }
			void set(double num) { m_time = num; OnPropertyChanged("Time"); }
		}

		// Унаследовано через INotifyPropertyChanged
		virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler ^ PropertyChanged;
		// продолжительность в секундах, опять же для удобства. Будем вычислять автоматичеки, проходясь по тексту и учитывая STempo
	};

	public ref class STempo sealed : public SketchMusic::ISymbol
	{
	public:
		STempo() {}
		STempo(float t) { value = t; }

		property float value;

		virtual SymbolType GetSymType() { return SymbolType::TEMPO; }
		virtual Platform::String^ ToString() { return value.ToString(); }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
	};

	public ref class SRNote sealed : public ISymbol, public SketchMusic::INote
	{
	private:
		static Platform::String^ valToString(int val);
		// 0 = тоника, далее по полутонам
		// положение тоники будет задаваться с помощью символов SScale
	public:
		SRNote() { _val = 0; }
		SRNote(int rtone) { _val = rtone; _velocity = 0; _voice = 0; }
		SRNote(int rtone, int velocity, int voice) { _val = rtone; _velocity = velocity; _voice = voice; }

		virtual property int _val;
		virtual property int _velocity;
		virtual property int _voice;

		virtual SymbolType GetSymType() { return SymbolType::RNOTE; }
		virtual Platform::String^ ToString() { return valToString(_val); }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "rnote," + valToString(_val); }
	};

	public ref class SGNote sealed : public ISymbol, public SketchMusic::INote
	{
	private:
		static Platform::String^ valToString(int val);
		// никак не отсчитывается.
		// в дальнейшем для обеспечения автоматической конкретизации будет зависимым от "паттерна" (?)	
	public:
		SGNote() {}
		SGNote(int gnote) { _val = gnote; _velocity = 0; _voice = 0; }
		SGNote(int gtone, int velocity, int voice) { _val = gtone; _velocity = velocity; _voice = voice; }

		virtual property int _val
		{
			int get() { return (abs(_valX) + _valY * 100)* ((_valX >= 0 )? 1 : -1 ); }
			void set(int val) { _valY = abs(val) / 100; _valX = val % 100; }
		}
		
		property int _valX;
		property int _valY;
		virtual property int _velocity;
		virtual property int _voice;

		virtual SymbolType GetSymType() { return SymbolType::GNOTE; }
		virtual Platform::String^ ToString() { return L"" + _valX + ";" + _valY; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "gnote," + valToString(_val); }
	};

	/**
	Заканчивает все ноты, какие есть в этой дорожке
	*/
	public ref class SNoteEnd sealed : public ISymbol, INote
	{
	public:
		SNoteEnd() {}

		virtual SymbolType GetSymType() { return SymbolType::END; }
		virtual Platform::String^ ToString() { return L"\uE81A"; }

		// Унаследовано через INote
		virtual property int _val;
		virtual property int _velocity;
		virtual property int _voice;

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "end"; }
	};

	// Структура, определяющая модификаторы аккорда
	public value struct SChordMod
	{
		bool minor;	// = _3dim; 0 = major
		bool dim;	// = _5dim; 0 = normal
		bool aug;	// = _5aug; 0 = normal. dim && aug == normal

		// модификаторы аккорда - включение (или выключение) ступеней в аккорд
		bool _2;
		bool _3ex;	// ВЫКЛЮЧАЕТ ступень из аккорда. Нужно для получения нетерцовых аккордов
		bool _4;
		bool _5ex;	// ВЫКЛЮЧАЕТ ступень из аккорда.
		bool _6;
		bool _7;
		bool _8;
		bool _9;

		// "небазовые" модуляторы звукоряда - можно изменять только ступени 2..7
		// 1 всегда остаётся 1, вне октавы всё повторяется. 7aug = oct = 1 пренебрегаем
		bool _2dim;
		bool _2aug;
		bool _3aug;
		bool _4dim;
		bool _4aug;
		bool _6dim;
		bool _6aug;
		bool dom;		// = _7dim; "доминантность" 7аккорда. Вне зависимости от _7 изменяет положение ступени звукоряда
	};

	/**
	Определяет текущую гамму
	- нужен класс Конкретизатор
	*/
	public ref class SChord sealed : public ISymbol //, INote
	{
	private:
		bool ModulatorsEQ(SChordMod m);

	public:
		SChord() {}
		SChord(int _key, int _deg, int _var, int _bass, SChordMod _mod)
		{
			key = _key; degree = _deg; variation = _var; bass = _bass; mod = _mod;
		}

		virtual SymbolType GetSymType() { return SymbolType::CHORD; }
		virtual Platform::String^ ToString() { return "SChord"; }

		property int key;			// В каком ключе будет аккорд: C, D, E ... Тут будет содержаться значение конкретной ноты
		property int degree;		// ступень гаммы, от которой отстраивается аккорд: I, II, V ... соответсвует также ладу: ионийский, ...
		property int variation;		// обращение аккорда. Нужно будет для конкретизации. Не знаю пока, как учитывать широкое / узкое расположение
		property int bass;			// значение ноты в басу

		property SChordMod mod;	// модификаторы

								// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);

		// Унаследовано через INote
		//virtual property int _val;
		//virtual property int _velocity;
		//virtual property int _voice;
	};

	// переименовать в LinStart и пусть он хранит всю необходимую информацию
	// для текущей строки, чтобы можно было её безболезненно вытащить из общего текста,
	// а ещё чтобы быстро можно было осуществлять конкретизацию
	// Хотя лучше просто в начало строки ставить все необходимые символы, 
	// а этот использовать только для форматирования.
	public ref class SNewLine sealed : public ISymbol
	{
	public:
		SNewLine() {}

		virtual SymbolType GetSymType() { return SymbolType::NLINE; }
		virtual Platform::String^ ToString() { return L"\uE751"; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "nline"; }
	};

	/**
	Чтобы можно было вставлять текст в композицию
	*/
	public ref class SString sealed : public ISymbol
	{
	public:
		SString() {}

		property String^ value;

		virtual SymbolType GetSymType() { return SymbolType::STRING; }
		virtual Platform::String^ ToString() { return value; }

		// Унаследовано через ISymbol
		virtual bool EQ(ISymbol ^ second);
		//virtual Platform::String^ Serialize() { return "string," + value; }
	};

	namespace SerializationTokens
	{
		static Platform::String^ TEXT = "text";		// чтобы можно было в одну строку несколько текстов
		static Platform::String^ INSTR = "instr";		// название инструмента
		static Platform::String^ INSTR_FNAME = "file";		// имя файла с инструментом
		static Platform::String^ INSTR_PRESET = "pset";		// название инструмента
		static Platform::String^ BEAT = "p1";			// beat
		static Platform::String^ TICK = "p2";			// tick
		static Platform::String^ SYMBOL_TYPE = "t";		// тип символа
		static Platform::String^ NOTE_VALUE = "v";		// значение
		static Platform::String^ NOTE_VELOCITY = "y";	// динамика
		static Platform::String^ NOTE_VOICE = "o";		// номер голоса
		static Platform::String^ NOTES_ARRAY = "notes";	// обозначение массива с нотами
		static Platform::String^ CONTROL_TEXT = "ctrl_txt";	// обозначение текста, содержащего общие для всех "инструментальных" текстов символы
		static Platform::String^ TEXTS_ARRAY = "texts";	// обозначение массива с текстами
		static Platform::String^ PROJ_NAME = "name";	// имя проекта
		static Platform::String^ PROJ_DESC = "descr";	// описание проекта
		static Platform::String^ PROJ_BPM = "bpm";		// базовый темп проекта
		static Platform::String^ PART_NAME = "n";		// название части
		static Platform::String^ PART_CAT = "s";		// категория части
		static Platform::String^ PART_DYN = "d";		// динамика части

	}

	// классы для обеспечения модели синтеза SoundFont
	// Наберём все возможные, потом выпилим ненужне
	namespace SFReader{

		ref class SFInstrument;
		ref class SFInstrumentZone;
		ref class SFPresetZone;
		ref class SFPreset;
		ref class SFData;
		ref class SFSample;
		ref class SoundFont;
		ref class SFGenerator;
		ref class SFModulator;
		ref class SFVersion;
		ref class SFLoader;
		ref class SFChunk;

		public enum class SFGeneratorID
		{
			startAddrsOffset = 0,	// The offset, in sample data points, beyond the Start sample header parameter to the first sample data point to be played for this instrument.For example, if Start were 7 and startAddrOffset were 2, the first sample data point played would be sample data point 9.
			endAddrsOffset,			// The offset, in sample sample data points, beyond the End sample header parameter to the last sample data point to be played for this instrument.For example, if End were 17 and endAddrOffset were - 2, the last sample data point played would be sample data point 15.
			startloopAddrsOffset,	// The offset, in sample data points, beyond the Startloop sample header parameter to the first sample data point to be repeated in the loop for this instrument.For example, if Startloop were 10 and startloopAddrsOffset were - 1, the first repeated loop sample data point would be sample data point 9.
			endloopAddrsOffset,		// The offset, in sample data points, beyond the Endloop sample header parameter to the sample data point considered equivalent to the Startloop sample data point for the loop for this instrument.For example, if Endloop were 15 and endloopAddrsOffset were 2, sample data point 17 would be considered equivalent to the Startloop sample data point, and hence sample data point 16 would effectively precede Startloop during looping.
			startAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Start sample header parameter and the first sample data point to be played in this instrument.This parameter is added to the startAddrsOffset parameter.For example, if Start were 5, startAddrsOffset were 3 and startAddrsCoarseOffset were 2, the first sample data point played would be sample data point 65544.
			modLfoToPitch,			// This is the degree, in cents, to which a full scale excursion of the Modulation LFO will influence pitch.A positive value indicates a positive LFO excursion increases pitch; a negative value indicates a positive excursion decreases pitch.Pitch is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 100 indicates that the pitch will first rise 1 semitone, then fall one semitone.
			vibLfoToPitch,			// This is the degree, in cents, to which a full scale excursion of the Vibrato LFO will influence pitch.A positive value indicates a positive LFO excursion increases pitch; a negative value indicates a positive excursion decreases pitch.Pitch is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 100 indicates that the pitch will first rise 1 semitone, then fall one semitone.
			modEnvToPitch,			// This is the degree, in cents, to which a full scale excursion of the Modulation Envelope will influence pitch.A positive value indicates an increase in pitch; a negative value indicates a decrease in pitch.Pitch is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 100 indicates that the pitch will rise 1 semitone at the envelope peak.
			initialFilterFc,		// This is the cutoff and resonant frequency of the lowpass filter in absolute cent units.The lowpass filter is defined as a second order resonant pole pair whose pole frequency in Hz is defined by the Initial Filter Cutoff parameter.When the cutoff frequency exceeds 20kHz and the Q(resonance) of the filter is zero, the filter does not affect the signal.
			initialFilterQ,			// This is the height above DC gain in centibels which the filter resonance exhibits at the cutoff frequency.A value of zero or less indicates the filter is not resonant; the gain at the cutoff frequency(pole angle) may be less than zero when zero is specified.The filter gain at DC is also affected by this parameter such that the gain at DC is reduced by half the specified gain.For example, for a value of 100, the filter gain at DC would be 5 dB below unity gain, and the height of the resonant peak would be 10 dB above the DC gain, or 5 dB above unity gain.Note also that if initialFilterQ is set to zero or less and the cutoff frequency exceeds 20 kHz, then the filter response is flat and unity gain.
			modLfoToFilterFc,		// This is the degree, in cents, to which a full scale excursion of the Modulation LFO will influence filter cutoff frequency.A positive number indicates a positive LFO excursion increases cutoff frequency; a negative number indicates a positive excursion decreases cutoff frequency.Filter cutoff frequency is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 1200 indicates that the cutoff frequency will first rise 1 octave, then fall one octave.
			modEnvToFilterFc,		// This is the degree, in cents, to which a full scale excursion of the Modulation Envelope will influence filter cutoff frequency.A positive number indicates an increase in cutoff frequency; a negative number indicates a decrease in filter cutoff frequency.Filter cutoff frequency is always modified logarithmically, that is the deviation is in cents, semitones, and octaves rather than in Hz.For example, a value of 1000 indicates that the cutoff frequency will rise one octave at the envelope attack peak.
			endAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the End sample header parameter and the last sample data point to be played in this instrument.This parameter is added to the endAddrsOffset parameter.For example, if End were 65536, startAddrsOffset were - 3 and startAddrsCoarseOffset were - 1, the last sample data point played would be sample data point 32765.
			modLfoToVolume,			// This is the degree, in centibels, to which a full scale excursion of the Modulation LFO will influence volume.A positive number indicates a positive LFO excursion increases volume; a negative number indicates a positive excursion decreases volume.Volume is always modified logarithmically, that is the deviation is in decibels rather than in linear amplitude.For example, a value of 100 indicates that the volume will first rise ten dB, then fall ten dB.
			unused1,				// Unused, reserved.Should be ignored if encountered.
			chorusEffectsSend,		// This is the degree, in 0.1% units, to which the audio output of the note is sent to the chorus effects processor.A value of 0 % or less indicates no signal is sent from this note; a value of 100 % or more indicates the note is sent at full level.Note that this parameter has no effect on the amount of this signal sent to the “dry” or unprocessed portion of the output.For example, a value of 250 indicates that the signal is sent at 25 % of full level(attenuation of 12 dB from full level) to the chorus effects processor.
			reverbEffectsSend,		// This is the degree, in 0.1% units, to which the audio output of the note is sent to the reverb effects processor.A value of 0 % or less indicates no signal is sent from this note; a value of 100 % or more indicates the note is sent at full level.Note that this parameter has no effect on the amount of this signal sent to the “dry” or unprocessed portion of the output.For example, a value of 250 indicates that the signal is sent at 25 % of full level(attenuation of 12 dB from full level) to the reverb effects processor.
			pan,					// This is the degree, in 0.1% units, to which the “dry” audio output of the note is positioned to the left or right output.A value of - 50 % or less indicates the signal is sent entirely to the left output and not sent to the right output; a value of + 50 % or more indicates the note is sent entirely to the right and not sent to the left.A value of zero places the signal centered between left and right.For example, a value of - 250 indicates that the signal is sent at 75 % of full level to the left output and 25 % of full level to the right output.
			unused2,				// Unused, reserved.Should be ignored if encountered.
			unused3,				// Unused, reserved.Should be ignored if encountered.
			unused4,				// Unused, reserved.Should be ignored if encountered.
			delayModLFO,			// This is the delay time, in absolute timecents, from key on until the Modulation LFO begins its upward ramp from zero value.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second and a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = 7973.
			freqModLFO,				// This is the frequency, in absolute cents, of the Modulation LFO’s triangular period.A value of zero indicates a frequency of 8.176 Hz.A negative value indicates a frequency less than 8.176 Hz; a positive value a frequency greater than 8.176 Hz.For example, a frequency of 10 mHz would be 1200log2(.01 / 8.176) = -11610.
			delayVibLFO,			// This is the delay time, in absolute timecents, from key on until the Vibrato LFO begins its upward ramp from zero value.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
			freqVibLFO,				// This is the frequency, in absolute cents, of the Vibrato LFO’s triangular period.A value of zero indicates a frequency of 8.176 Hz.A negative value indicates a frequency less than 8.176 Hz; a positive value a frequency greater than 8.176 Hz.For example, a frequency of 10 mHz would be 1200log2(.01 / 8.176) = -11610.
			delayModEnv,			// This is the delay time, in absolute timecents, between key on and the start of the attack phase of the Modulation envelope.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
			attackModEnv,			// This is the time, in absolute timecents, from the end of the Modulation Envelope Delay Time until the point at which the Modulation Envelope value reaches its peak.Note that the attack is “convex”; the curve is nominally such that when applied to a decibel or semitone parameter, the result is linear in amplitude or Hz respectively.A value of 0 indicates a 1 second attack time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates instantaneous attack.For example, an attack time of 10 msec would be 1200log2(.01) = -7973.
			holdModEnv,				// This is the time, in absolute timecents, from the end of the attack phase to the entry into decay phase, during which the envelope value is held at its peak.A value of 0 indicates a 1 second hold time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(32768) conventionally indicates no hold phase.For example, a hold time of 10 msec would be 1200log2(.01) = -7973.
			decayModEnv,			// This is the time, in absolute timecents, for a 100 % change in the Modulation Envelope value during decay phase.For the Modulation Envelope, the decay phase linearly ramps toward the sustain level.If the sustain level were zero, the Modulation Envelope Decay Time would be the time spent in decay phase.A value of 0 indicates a 1 second decay time for a zero - sustain level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a decay time of 10 msec would be 1200log2(.01) = -7973.
			sustainModEnv,			// This is the decrease in level, expressed in 0.1% units, to which the Modulation Envelope value ramps during the decay phase.For the Modulation Envelope, the sustain level is properly expressed in percent of full scale.Because the volume envelope sustain level is expressed as an attenuation from full scale, the sustain level is analogously expressed as a decrease from full scale.A value of 0 indicates the sustain level is full level; this implies a zero duration of decay phase regardless of decay time.A positive value indicates a decay to the corresponding level.Values less than zero are to be interpreted as zero; values above 1000 are to be interpreted as 1000.  For example, a sustain level which corresponds to an absolute value 40 % of peak would be 600.
			releaseModEnv,			// This is the time, in absolute timecents, for a 100 % change in the Modulation Envelope value during release phase.For the Modulation Envelope, the release phase linearly ramps toward zero from the current level.If the current level were full scale, the Modulation Envelope Release Time would be the time spent in release phase until zero value were reached.A value of 0 indicates a 1 second decay time for a release from full level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a release time of 10 msec would be 1200log2(.01) = -7973.
			keynumToModEnvHold,		// This is the degree, in timecents per KeyNumber units, to which the hold time of the Modulation Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Modulation Envelope Hold Time were - 7973 = 10 msec and the Key Number to Mod Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
			keynumToModEnvDecay,	// This is the degree, in timecents per KeyNumber units, to which the hold time of the Modulation Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Modulation Envelope Hold Time were - 7973 = 10 msec and the Key Number to Mod Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
			delayVolEnv,			// This is the delay time, in absolute timecents, between key on and the start of the attack phase of the Volume envelope.A value of 0 indicates a 1 second delay.A negative value indicates a delay less than one second; a positive value a delay longer than one second.The most negative number(-32768) conventionally indicates no delay.For example, a delay of 10 msec would be 1200log2(.01) = -7973.
			attackVolEnv,			// This is the time, in absolute timecents, from the end of the Volume Envelope Delay Time until the point at which the Volume Envelope value reaches its peak.Note that the attack is “convex”; the curve is nominally such that when applied to the decibel volume parameter, the result is linear in amplitude.A value of 0 indicates a 1 second attack time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates instantaneous attack.For example, an attack time of 10 msec would be 1200log2(.01) = -7973.
			holdVolEnv,				// This is the time, in absolute timecents, from the end of the attack phase to the entry into decay phase, during which the Volume envelope value is held at its peak.A value of 0 indicates a 1 second hold time.A negative value indicates a time less than one second; a positive value a time longer than one second.The most negative number(-32768) conventionally indicates no hold phase.For example, a hold time of 10 msec would be 1200log2(.01) = -7973.
			decayVolEnv,			// This is the time, in absolute timecents, for a 100 % change in the Volume Envelope value during decay phase.For the Volume Envelope, the decay phase linearly ramps toward the sustain level, causing a constant dB change for each time unit.If the sustain level were - 100dB, the Volume Envelope Decay Time would be the time spent in decay phase.A value of 0 indicates a 1 - second decay time for a zero - sustain level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a decay time of 10 msec would be 1200log2(.01) = -7973.
			sustainVolEnv,			// This is the decrease in level, expressed in centibels, to which the Volume Envelope value ramps during the decay phase.For the Volume Envelope, the sustain level is best expressed in centibels of attenuation from full scale.A value of 0 indicates the sustain level is full level; this implies a zero duration of decay phase regardless of decay time.A positive value indicates a decay to the corresponding level.Values less than zero are to be interpreted as zero; conventionally 1000 indicates full attenuation.For example, a sustain level which corresponds to an absolute value 12dB below of peak would be 120.
			releaseVolEnv,			// This is the time, in absolute timecents, for a 100 % change in the Volume Envelope value during release phase.For the Volume Envelope, the release phase linearly ramps toward zero from the current level, causing a constant dB change for each time unit.If the current level were full scale, the Volume Envelope Release Time would be the time spent in release phase until 100dB attenuation were reached.A value of 0 indicates a 1 - second decay time for a release from full level.A negative value indicates a time less than one second; a positive value a time longer than one second.For example, a release time of 10 msec would be 1200log2(.01) = -7973.
			keynumToVolEnvHold,		// This is the degree, in timecents per KeyNumber units, to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Volume Envelope Hold Time were - 7973 = 10 msec and the Key Number to Vol Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
			keynumToVolEnvDecay,	// This is the degree, in timecents per KeyNumber units, to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.The hold time at key number 60 is always unchanged.The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard; that is, an upward octave causes the hold time to halve.For example, if the Volume Envelope Hold Time were - 7973 = 10 msec and the Key Number to Vol Env Hold were 50 when key number 36 was played, the hold time would be 20 msec.
			instrument,				// This is the index into the INST sub - chunk providing the instrument to be used for the current preset zone.A value of zero indicates the first instrument in the list.The value should never exceed two less than the size of the instrument list.The instrument enumerator is the terminal generator for PGEN zones.As such, it should only appear in the PGEN sub - chunk, and it must appear as the last generator enumerator in all but the global preset zone.
			reserved1,				// Unused, reserved.Should be ignored if encountered.
			keyRange,				// This is the minimum and maximum MIDI key number values for which this preset zone or instrument zone is active.The LS byte indicates the highest and the MS byte the lowest valid key.The keyRange enumerator is optional, but when it does appear, it must be the first generator in the zone generator list.
			velRange,				// This is the minimum and maximum MIDI velocity values for which this preset zone or instrument zone is active.The LS byte indicates the highest and the MS byte the lowest valid velocity.The velRange enumerator is optional, but when it does appear, it must be preceded only by keyRange in the zone generator list.
			startloopAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Startloop sample header parameter and the first sample data point to be repeated in this instrument’s loop.This parameter is added to the startloopAddrsOffset parameter.For example, if Startloop were 5, startloopAddrsOffset were 3 and startAddrsCoarseOffset were 2, the first sample data point in the loop would be sample data point 65544.
			keynum,					// This enumerator forces the MIDI key number to effectively be interpreted as the value given.This generator can only appear at the instrument level.Valid values are from 0 to 127.
			velocity,				// This enumerator forces the MIDI velocity to effectively be interpreted as the value given.This generator can only appear at the instrument level.Valid values are from 0 to 127.
			initialAttenuation,		// This is the attenuation, in centibels, by which a note is attenuated below full scale.A value of zero indicates no attenuation; the note will be played at full scale.For example, a value of 60 indicates the note will be played at 6 dB below full scale for the note.
			reserved2,				// Unused, reserved.Should be ignored if encountered.
			endloopAddrsCoarseOffset,	// The offset, in 32768 sample data point increments beyond the Endloop sample header parameter to the sample data point considered equivalent to the Startloop sample data point for the loop for this instrument.This parameter is added to the endloopAddrsOffset parameter.For example, if Endloop were 5, endloopAddrsOffset were 3 and endAddrsCoarseOffset were 2, sample data point 65544 would be considered equivalent to the Startloop sample data point, and hence sample data point 65543 would effectively precede Startloop during looping.
			coarseTune,				// This is a pitch offset, in semitones, which should be applied to the note.A positive value indicates the sound is reproduced at a higher pitch; a negative value indicates a lower pitch.For example, a Coarse Tune value of - 4 would cause the sound to be reproduced four semitones flat.
			fineTune,				// This is a pitch offset, in cents, which should be applied to the note.It is additive with coarseTune.A positive value indicates the sound is reproduced at a higher pitch; a negative value indicates a lower pitch.For example, a Fine Tuning value of 5 would cause the sound to be reproduced five cents flat.
			sampleID,				// This is the index into the SHDR sub - chunk providing the sample to be used for the current instrument zone.A value of zero indicates the first sample in the list.The value should never exceed two less than the size of the sample list.The sampleID enumerator is the terminal generator for IGEN zones.As such, it should only appear in the IGEN sub - chunk, and it must appear as the last generator enumerator in all but the global zone.
			sampleModes,			// This enumerator indicates a value which gives a variety of Boolean flags describing the sample for the current instrument zone.The sampleModes should only appear in the IGEN sub - chunk, and should not appear in the global zone.The two LS bits of the value indicate the type of loop in the sample : 0 indicates a sound reproduced with no loop, 1 indicates a sound which loops continuously, 2 is unused but should be interpreted as indicating no loop, and 3 indicates a sound which loops for the duration of key depression then proceeds to play the remainder of the sample.
			reserved3,				// Unused, reserved.Should be ignored if encountered.
			scaleTuning,			// This parameter represents the degree to which MIDI key number influences pitch.A value of zero indicates that MIDI key number has no effect on pitch; a value of 100 represents the usual tempered semitone scale.
			exclusiveClass,			// This parameter provides the capability for a key depression in a given instrument to terminate the playback of other instruments.This is particularly useful for percussive instruments such as a hi - hat cymbal.An exclusive class value of zero indicates no exclusive class; no special action is taken.Any other value indicates that when this note is initiated, any other sounding note with the same exclusive class value should be rapidly terminated.The exclusive class generator can only appear at the instrument level.The scope of the exclusive class is the entire preset.In other words, any other instrument zone within the same preset holding a corresponding exclusive class will be terminated.
			overridingRootKey,		// This parameter represents the MIDI key number at which the sample is to be played back at its original sample rate.If not present, or if present with a value of - 1, then the sample header parameter Original Key is used in its place.If it is present in the range 0 - 127, then the indicated key number will cause the sample to be played back at its sample header Sample Rate.For example, if the sample were a recording of a piano middle C(Original Key = 60) at a sample rate of 22.050 kHz, and Root Key were set to 69, then playing MIDI key number 69 (A above middle C) would cause a piano note of pitch middle C to be heard.
			unused5,				// Unused, reserved.Should be ignored if encountered.
			endOper					// Unused, reserved.Should be ignored if encountered.Unique name provides value to end of defined list
		};

		union SFGenValue
		{
			signed short sword;			// signed 16 bit value
			unsigned short uword;		// unsigned 16 bit value
			struct
			{
				unsigned char lo;			// low value for ranges
				unsigned char hi;			// high value for ranges
			}
			range;
		};

		ref class SFGenAmount sealed
		{				// Generator amount structure
		internal:
			SFGenAmount() { val.sword = 0; }

			SFGenValue val;
		};

		// пока добавим такие вот классы, потом посмотрим, можно ли их будет
		// совместить с уже существующими
		//ref class SFSoundEngine;
		ref class SFVoice;
	}

	namespace Commands
	{
		public delegate bool CanExecuteMethod(Object^);


		// === определения функций-делегатов для обеспечения команд



		// управление долями - перемещение сразу многих символов
		// void InsertBeat(Text, Position, Count?) // вместо Count можно использовать Position - удаление диапазона
		// void DeleteBeat(Text, Position, Count?)

		public ref class CompositionHandlerArgs sealed
		{
			SketchMusic::Composition^ composition;	// TODO : заменить на Composition
			String^ fileName;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class SymbolHandlerArgs sealed
		{
		public:
			SymbolHandlerArgs(Text^ text, PositionedSymbol^ sym)
			{
				_text = text;
				_sym = sym;
			}

			property Text^ _text;
			property PositionedSymbol^ _sym;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class SymbolPairHandlerArgs sealed
		{
		public:
			// Конструктор для операций с двумя символами (перемещение, удаление в диапазоне)
			SymbolPairHandlerArgs(Text^ text, PositionedSymbol^ oldSym, PositionedSymbol^ newSym)
			{
				_text = text;
				_oldSym = oldSym;
				_newSym = newSym;
			}

			property Text^ _text;
			property PositionedSymbol^ _oldSym;
			property PositionedSymbol^ _newSym;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class MultiSymbolHandlerArgs sealed
		{
		public:
			// Конструктор для операций с одним символом (добавление/удаление)
			MultiSymbolHandlerArgs(Text^ text, IVector<PositionedSymbol^>^ sym)
			{
				_text = text;
				_OldSymbols = sym;
			}

			// Конструктор для операций с двумя символами (перемещение, удаление в диапазоне)
			MultiSymbolHandlerArgs(Text^ text, IVector<PositionedSymbol^>^ oldSym, IVector<PositionedSymbol^>^ newSym)
			{
				_text = text;
				_OldSymbols = oldSym;
				_NewSymbols = newSym;
			}

			property Text^ _text;
			property IVector<PositionedSymbol^>^ _OldSymbols;
			property IVector<PositionedSymbol^>^ _NewSymbols;
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		// Вставка/удаление долей
		public ref class BeatHandlerArgs sealed
		{
			Text^ text;
			Cursor^ position;
			int count;
		};

		// Необходимые команды по работе с текстом
		public delegate bool Handler(Object^ args); // bool = выполнено или нет

		// сохранение-загрузка
		// void Save(Text, FileName, Options?)
		// Text Load(FileName, Options?) -> void Load( ..Text^.. ) - интересно, можно ли так
		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public delegate bool CompositionHandler(CompositionHandlerArgs^ args);

		// управление символами
		// void Insert(Text, PositionedSymbol)
		// void Move(Text, PositionedSymbol, Position newPos)
		// void Delete(Text, PositionedSymbol)
		// void Edit(Text, PositionedSymbol, Symbol newSym) // вместо комбинации delete-insert
		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public delegate bool SymbolHandler(SymbolHandlerArgs^ args);


		// основные классы
		//ref class Command;			// с делегатами в качестве исполняемых функций
		public ref class Command sealed
		{
		private:
			Handler^ _Handler;
			Handler^ _Unexecute;
			CanExecuteMethod^ _CanExecute;

		public:
			Command(Handler^ handler, Handler^ unexecute)
			{
				_Handler = handler;
				_Unexecute = unexecute;
			}

			Command(Handler^ handler, Handler^ unexecute, CanExecuteMethod^ canExecute)
			{
				_Handler = handler;
				_CanExecute = canExecute;
				_Unexecute = unexecute;
			}

		internal:

			virtual bool CanExecute(Object^ parameter)
			{
				if (_CanExecute)
					return _CanExecute(parameter);
				return true;
			}

			virtual bool Execute(Object^ parameter)
			{
				if (_Handler) // && CanExecute
					return _Handler(parameter);
				return false;
			}

			virtual bool Unexecute(Object^ parameter)
			{
				if (_Unexecute)
					return _Unexecute(parameter);
				return false;
			}

			virtual event EventHandler<Platform::Object^>^ CanExecuteChanged;
		};

		public ref class CommandState sealed		// команда вместе с её параметром
		{
		public:
			CommandState(Command^ command, Object^ args)
			{
				_command = command;
				_args = args;
			}

			// обёртка, чтобы не морочиться с аргументами
			// TODO : возвращать bool - выполнена команда или нет
			bool Execute() { return _command->Execute(_args); }
			bool Unexecute() { return _command->Unexecute(_args); }

			property Command^ _command;
			property Object^ _args;
		};

		const int DEFAULT_MAX_COM = 100;
		
		// принимает команды и хранит историю
		//[Windows::UI::Xaml::Data::Bindable]
		public ref class CommandManager sealed //: Windows::UI::Xaml::Data::INotifyPropertyChanged
		{
		private:
			// История команд
			Windows::Foundation::Collections::IVector<SketchMusic::Commands::CommandState^>^ _commands;
			//void OnPropertyChanged(Platform::String^ propertyName);
			int m_index;
			bool m_canUndo;
			bool m_canRedo;

		public:

			//virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;

			// делаем вручную, потому что мне не удалось заставить работать привязки через ProertyChanged
			event EventHandler<bool>^ CanUndoChanged;	
			event EventHandler<bool>^ CanRedoChanged;

			CommandManager()
			{
				MaxCommandCount = DEFAULT_MAX_COM;
				_commands = ref new Platform::Collections::Vector<CommandState^>;
				_CurrentIndex = -1;
			}

			CommandManager(int max)
			{
				MaxCommandCount = max;
				_commands = ref new Platform::Collections::Vector<CommandState^>;
				_CurrentIndex = -1;
			}

			property int MaxCommandCount; // максимальное количество команд
			property int _CurrentIndex	// указывает индекс последней выполненной команды (-1, если ничего нет)
			{
				int get() { return m_index; }
				void set(int ind)
				{ 
					m_index = ind;
					if (m_index >= 0) 
						CanUndo = true;
					else CanUndo = false;
					//int max = ; // по непонятной причине, если просто вставить выражение в возвращаемое значение,
					if (m_index < ((int)_commands->Size - 1))
						CanRedo = true;
					else CanRedo = false;
				}
			}
			property bool CanUndo { bool get() { return m_canUndo; }
				void set(bool can) 
				{
					if (m_canUndo == can) return;
					m_canUndo = can;
					//OnPropertyChanged(L"CanUndo");
					CanUndoChanged(this, m_canUndo);
				} 
			}
			property bool CanRedo
			{
				bool get() { return m_canRedo; }
				void set(bool can)
				{
					if (m_canRedo == can) return;
					m_canRedo = can;
					//OnPropertyChanged(L"CanRedo");
					CanRedoChanged(this, m_canRedo);
				}
			}

			// список доступных манипуляций
			bool Undo()
			{
				if (CanUndo)
				{
					// "разделываем" последнюю выполненую команду, уменьшаем счётчик
					return _commands->GetAt(_CurrentIndex--)->Unexecute();
				};
				return false;
			}

			bool Redo()
			{
				if (CanRedo)
				{
					return ExecuteCurrent();
				}
				return false;
			}

			void Clear()
			{
				_commands->Clear();
				_CurrentIndex = -1;
			}
			
			// Выполняет текущую команду (последнюю или ту, на которую мы отмотали в результате undo-redo)
			bool ExecuteCurrent()
			{
				// переводим счётчик на следующую команду, делаем её
				return _commands->GetAt(++_CurrentIndex)->Execute();
			}

			bool AddAndExecute(SketchMusic::Commands::Command^ command, Platform::Object^ args)
			{
				auto commandState = ref new CommandState(command, args);
				// если мы посередине списка команд - убираем все последующие, чтобы не создавать параллельных веток
				if (_CurrentIndex != _commands->Size - 1)
				{
					for (int cur = _commands->Size - 1; cur > _CurrentIndex; cur--)
					{
						_commands->RemoveAtEnd();
					}
				}

				// если количество команд достигло заданного предела или по каким-то причинам превышает его
				// - удаляем команду из начала
				while (_CurrentIndex >= MaxCommandCount-1)
				{
					_commands->RemoveAt(0);
					_CurrentIndex--;
				}

				_commands->Append(commandState);
				return ExecuteCurrent();
			}
		};
	}

	namespace Player
	{
		// состояния плеера
		public enum class PlayerState
		{
			STOP = 0,	// стоит - совсем
			PLAY = 1,	// проигрывает
			WAIT = 2,	// останавливается - нужен для проверок на наличие нот. При их наличии продолжает играть
		};

		[Windows::Foundation::Metadata::WebHostHiddenAttribute]
		public ref class PlayerStateToPlayBtnContentConverter sealed : Windows::UI::Xaml::Data::IValueConverter
		{
		public:
			virtual Object^ Convert(Platform::Object ^value, Windows::UI::Xaml::Interop::TypeName targetType, Platform::Object ^parameter, Platform::String ^language)
			{
				SketchMusic::Player::PlayerState state = (SketchMusic::Player::PlayerState) value;
				switch (state)
				{
				case SketchMusic::Player::PlayerState::STOP:
					return "Воспроизвести";
				case SketchMusic::Player::PlayerState::PLAY:
				case SketchMusic::Player::PlayerState::WAIT:
					return "Остановить";
				}
				return nullptr;
			}

			virtual Object^ ConvertBack(Object^ value, Windows::UI::Xaml::Interop::TypeName  targetType, Object^ parameter, Platform::String^ language)
			{
				return nullptr;
			}

			PlayerStateToPlayBtnContentConverter() {}
		};


		ref class Player;
		interface class ISoundEngine;
		ref class SimpleSoundEngine;
		ref class SFSoundEngine;
		ref class SoundEnginePool;


		// переделать в NoteVoicing или как-нибудь так - обёртка вокруг голосов 
		// для управления параметрами звука в реальном времени
		public ref class NoteOff sealed
		{
		private:
			cancellation_token_source source;
		internal:
			concurrency::cancellation_token GetToken()
			{
				return source.get_token();
			}
		public:
			NoteOff() {}

			void Cancel()
			{
				source.cancel();
			}
		};
	}
	
	/**
	 * Пространство имён, объединяющее классы, отвечающие за графическое представление
	 * внутренних структур музыкального редактора.
	 */
	namespace View
	{
		/**
		Интерфейс, определяющий раскладку ... положение клавиш
		*/
		public interface class IKeyPosition
		{
			Windows::Foundation::Point GetKeyPosition(int i);
		};

		public enum class KeyType
		{
			note			= 0,	// "точная" нота
			relativeNote	= 1,	// относительная
			genericNote		= 2,	// "обобщённая"
			octave			= 3,	// сдвиг на октаву
			space			= 4,	// пробел, он же delete ?
			move			= 5,	// вместо left и right - на сколько надо сдвинуть будет определяться тегом
			enter			= 6,
			play			= 7,	// любое неотрицательное - плей, любое отрицательное - реверс плей
			stop			= 8,	// остановка плеера
			beat			= 9,
			shift			= 10,
			record			= 11,
			cycling			= 12,
			backspace		= 13,
			control			= 14,
			tempo			= 15,
			zoom			= 16,
			metronome		= 17,
			quantization	= 18,
			playGeneric		= 19,	// озвучиваем обобщённые ноты
			end				= 20,	// конец ноты - остановка звучания
			newPart			= 25,	// отметка о начале новой части
			hide			= 30,	// спрятать клавиатуру
			layout			= 31,	// сменить раскладку клавиатуры
			precount		= 32,	// установить предварительные тики перед проигрыванием
			accent			= 33,	// акцентирование
			eraser			= 35,	// "ластик"
		};

		public enum class KeyboardStateEnum
		{
			normal			= 0,
			pressed			= 1,	// для блокирования системных кнопок
			control			= 2,
			shift			= 3,
			octaveInc		= 4,
			octaveDec		= 5,
			play			= 6,	// для связи состояния плеера с состояниями клавиатуры
			stop			= 7,	// для связи состояния плеера с состояниями клавиатуры
			absoluteNote	= 10,	// для смены раскладок
			relativeNote	= 11,
			genericNote		= 12,
			specialSymbols	= 13,	// всякие вибрато, акценты и т.д.
			formatSymbols	= 14,
		};
		
		// на всякий случай обернём вышеупомянутое в класс
		// переделать в битовое поле?
		public ref class KeyboardState sealed
		{
		public:
			property KeyboardStateEnum state;

			KeyboardState(KeyboardStateEnum s) { state = s; }
		};

		ref class Key;
		
		public ref class KeyboardEventArgs sealed
		{
		private:
			SketchMusic::View::Key^ m_key;

		public:
			property SketchMusic::View::Key^ key
			{
				SketchMusic::View::Key^ get() {
					return m_key;
				}
			}
			property int keysPressed;

			KeyboardEventArgs(SketchMusic::View::Key^ _key, int _keysPressed) : m_key(_key)
			{
				keysPressed = _keysPressed;
			}
		};

		// не уверен, стоит ли выделять отдельный класс, но где-то это должно быть
		// Визуальное отображение (радиальные клавиатуры, с произвольным расположением клавиш и т.д.) будут соответсвовать разным классам элементов управления.
		// Каждый класс может использоваться для отображения любой (?) раскладки
		public enum class KeyboardType
		{
			Basic		= 0,	// просто
			Relative	= 1,	// подразумевается, что так будут вводиться "модулируемые" значения, абсолютное значение которых будет определяться "модуляторами"
			Generic		= 2,	// обобщённые ноты
			Classic		= 4,	// классическая клавиатура
			Modulators	= 5,	// "модуляторы" для "относительных" нот. Сейчас предполагается, что это будут значения двух видов - ступень текущей гаммы
								// или {ступень текущей; ступень следующей гаммы} - для переходов между гаммами - ступень одной становится ступенью другой	
								// ? будет полезно?
			Controls	= 10,	// надо было их сделать 0
		};
		
		// перечисление доступных типов отображения нот
		public enum class ViewType
		{
			TextRow		= 0,	// "обычное" представление нот
			ChordView	= 1,	// представление для последовательного ввода
		};

		public interface class IKeyboard
		{
			event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyPressed;	// нажатие на одну клавишу
			event EventHandler<SketchMusic::View::KeyboardEventArgs^>^ KeyReleased;	// нажатие на одну клавишу
			event EventHandler<Windows::Foundation::Collections::IVector<SketchMusic::View::Key^>^>^ KeyboardReleased; // "отпускание" всех клавиш
			event EventHandler<SketchMusic::View::KeyboardState^>^ KeyboardStateChanged;
		};

		// конвертеры значений
		ref class OnSymbolToStringConverter;
		ref class OnKeyToStringConverter;
		ref class TagToKeyConverter;		// как keytostring, только наоборот. Не понял как можно тот же конвертер в обратную сторону использовать
		ref class BoolToColorConverter;
		ref class BlackKeyToColorConverter;
		ref class KeyStateToColorConverter;
		ref class OnPositionedSymbolToTextConverter;
		ref class PSymbolToVerticalPosConverter;
		ref class PSymbolToHorizontalPosConverter;
		ref class PSymbolToStyleConverter;
		ref class MultiplicatedLengthConverter;
		ref class PSymbolPosToTextConverter;
		ref class PartCatToTextConverter;
		ref class PartDynToTextConverter;
		ref class PartTimeToTextConverter;
		ref class PartNameToTextConverter;
		ref class TestData;
		
		// варианты клавиатур
		//ref class RadialKeyboard;
		ref class BaseKeyboard;

		ref class TextRow;			// "строковое" представление музыкального текста
		ref class ChordView;		// для отображения нескольких нот
		ref class CompositionView;	// для отображения композиции в целом
	}

	[Windows::Foundation::Metadata::WebHostHiddenAttribute]
	public ref class ISymbolFactory sealed
	{
	public:
		static ISymbol^ Deserialize(Windows::Data::Json::JsonObject^ val);	// принудительно запрашиваем
		static ISymbol^ CreateSymbol(SketchMusic::View::KeyType type, Object^ val);
	};
}