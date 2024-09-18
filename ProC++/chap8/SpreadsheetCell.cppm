export module spreadsheet_cell;

export class SpreadsheetCell
{
    public: 
        void setValue(double value); //{ mValue = value;}
        double getValue() const;// { return mValue;}
    private:
        double mValue { 0 };
};