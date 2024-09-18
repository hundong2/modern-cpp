module spreadsheet_cell;

void SpreadsheetCell::setValue(double value) { mValue = value; }
double SpreadsheetCell::getValue() const { return mValue; }