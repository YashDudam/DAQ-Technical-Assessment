import "./App.css";

interface TemperatureProps {
  temp: number;
}

function LiveValue({ temp }: TemperatureProps) {
  let valueColour;
  if (temp < 20 || temp > 80) {
    valueColour = "red";
  } else if ((temp >= 20 && temp < 25) || (temp <= 80 && temp > 75)) {
    valueColour = "yellow";
  } else {
    valueColour = "green";
  }

  return (
    <header className="live-value" style={{ color: valueColour }}>
      {`${temp.toPrecision(3)}°C`}
    </header>
  );
}

export default LiveValue;
