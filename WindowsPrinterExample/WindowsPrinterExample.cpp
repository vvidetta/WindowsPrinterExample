#include <iostream>
#include "Windows.h"
#include <vector>

int main()
{
  auto buffer = std::vector<BYTE>{};
  auto bytesNeeded = DWORD{};
  auto numberOfEntries = DWORD{};
  if (!EnumPrinters(
    PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
    nullptr, 
    1,
    buffer.data(),
    buffer.size(),
    &bytesNeeded,
    &numberOfEntries))
  {
    buffer = std::vector<BYTE>(bytesNeeded);
    if (!EnumPrinters(
      PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS,
      nullptr,
      1,
      buffer.data(),
      buffer.size(),
      &bytesNeeded,
      &numberOfEntries))
    {
      std::wcerr << L"Failed to enumerate printers!" << std::endl;
      return 1;
    }
  }

  auto printers = std::vector<PRINTER_INFO_1>(numberOfEntries);
  std::memcpy(printers.data(), buffer.data(), numberOfEntries * sizeof(PRINTER_INFO_1));

  for (auto i = 0; i < printers.size(); ++i)
  {
    std::wcout << L"================\n"
      L"Printer " << i << "\n"
      L"Name: " << printers[i].pName << L"\n";
  }

  std::wcout << L"================\n"
    L"Enter printer number: ";

  int index;
  std::cin >> index;
  if (index < 0 || index >= printers.size())
  {
    std::wcerr << L"Invalid printer number\n";
    return 1;
  }

  auto const& printer_info = printers[index];
  auto handle = HANDLE{};

#define MESSAGE "Hello World!"
  auto textBuffer = std::vector<char>(std::size(MESSAGE));
  std::memcpy(textBuffer.data(), MESSAGE, sizeof(MESSAGE));
  auto bytesWritten = DWORD{};

  if (!OpenPrinter2(printer_info.pName, &handle, nullptr, nullptr))
  {
    std::wcerr << L"Failed to open printer" << std::endl;
    return 1;
  }
  int ret = 0;

  std::wcout << "Printer \"" << printer_info.pName << "\" Opened!" << std::endl;

  auto docName = std::vector<wchar_t>(11);
  std::memcpy(docName.data(), L"HelloWorld", sizeof(L"HelloWorld"));
#define RAW (L"RAW")
#define TEXT (L"TEXT")
#define ARRAYLEN(array) (sizeof(array) / sizeof((array)[0]))

  auto dataType = std::vector<wchar_t>(std::size(TEXT));
  std::memcpy(dataType.data(), TEXT, sizeof(TEXT));
  auto docInfo = DOC_INFO_1
  {
    docName.data(), // pDocName
    nullptr, // pOutputFile
    dataType.data() // pDataType
  };
  if (!StartDocPrinter(handle, 1, reinterpret_cast<BYTE*>(&docInfo)))
  {
    auto const err = GetLastError();
    std::wcerr << L"StartDocPrinter Failed with code " << err << std::endl;
    ret = 1;
    goto finish;
  }
  std::wcout << L"StartDocPrinter succeeded!" << std::endl;

  if (!StartPagePrinter(handle))
  {
    auto const err = GetLastError();
    std::wcerr << L"StartPagePrinter Failed with code " << err << std::endl;
    ret = 1;
    goto finish;
  }
  std::wcout << L"StartPagePrinter succeeded!" << std::endl;

  if (!WritePrinter(handle, textBuffer.data(), textBuffer.size(), &bytesWritten))
  {
    auto const err = GetLastError();
    std::wcerr << L"WritePrinter Failed with code " << err << std::endl;
    ret = 1;
    goto finish;
  }
  std::wcout << L"WritePrinter succeeded! Wrote " << bytesWritten << " bytes." << std::endl;

  if (!EndPagePrinter(handle))
  {
    auto const err = GetLastError();
    std::wcerr << L"EndPagePrinter Failed with code " << err << std::endl;
    ret = 1;
    goto finish;
  }
  std::wcout << L"EndPagePrinter succeeded!" << std::endl;

  if (!EndDocPrinter(handle))
  {
    std::wcerr << L"EndDocPrinter Failed" << std::endl;
    ret = 1;
    goto finish;
  }

  std::wcout << L"EndDocPrinter succeeded!" << std::endl;

finish:
  if (!ClosePrinter(handle))
  {
     std::wcerr << L"Failed to Close printer handler";
     return 1;
  }

  std::wcout << "Printer \"" << printer_info.pName << "\" Closed!" << std::endl;

  return ret;
}
