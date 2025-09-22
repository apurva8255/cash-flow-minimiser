from flask import Flask, render_template, request, jsonify
import subprocess
import os
import requests 

app = Flask(__name__)

# --- Gemini API Configuration ---
# In a real-world application, manage your API key securely.
# For this environment, we assume the key is handled by the execution environment.
GEMINI_API_KEY = "" # The execution environment provides the key.
GEMINI_API_URL = f"https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash-preview-05-20:generateContent?key={GEMINI_API_KEY}"

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/run", methods=["POST"])
def run_program():
    data = request.get_json()
    transactions = data.get('transactions', [])

    if not transactions:
        return jsonify({"error": "No transactions provided."})

    # Prepare input for the C program
    program_input = f"{len(transactions)}\n"
    for t in transactions:
        program_input += f"{t['payer']} {t['receiver']} {t['amount']}\n"
    
    # Define executable path
    executable_path = "./cashflow.exe"
    if not os.path.exists(executable_path):
         return jsonify({"error": "Error: cashflow.exe not found. Please compile CashFlow.c first using 'gcc CashFlow.c -o cashflow.exe -lm'"})

    try:
        # Run the compiled C program
        result = subprocess.check_output(
            [executable_path],
            input=program_input,
            text=True,
            encoding='utf-8',
            stderr=subprocess.STDOUT
        )
        return jsonify({"output": result})
    except subprocess.CalledProcessError as e:
        error_message = f"Error running the C program:\n{e.output}"
        return jsonify({"error": error_message})
    except Exception as e:
        return jsonify({"error": f"An unexpected error occurred: {str(e)}"})

@app.route("/generate-reminder", methods=["POST"])
def generate_reminder():
    data = request.get_json()
    settlements = data.get('settlements')
    tone = data.get('tone', 'Friendly')

    if not settlements:
        return jsonify({"error": "Settlement text is missing."})

    system_prompt = "You are a helpful assistant for a group of friends settling debts. Your task is to write a concise, clear, and appropriately toned message that can be sent in a group chat to remind people of the payments they need to make. Do not include any headers or introductory text like 'Here is the message:'. Just provide the message itself."
    user_query = f"Based on the following debt settlements, write a short, {tone.lower()} reminder message that can be sent in a group chat. Make sure to clearly list who needs to pay whom and the exact amount.\n\nSettlements:\n{settlements}"

    payload = {
        "contents": [{"parts": [{"text": user_query}]}],
        "systemInstruction": {"parts": [{"text": system_prompt}]},
    }
    
    try:
        response = requests.post(GEMINI_API_URL, json=payload, headers={'Content-Type': 'application/json'})
        response.raise_for_status() # Raises an exception for bad status codes (4xx or 5xx)
        
        result_json = response.json()
        candidate = result_json.get("candidates", [{}])[0]
        text = candidate.get("content", {}).get("parts", [{}])[0].get("text", "")

        if not text:
            return jsonify({"error": "Failed to generate reminder from API. The response was empty."})
            
        return jsonify({"reminder": text.strip()})
        
    except requests.exceptions.RequestException as e:
        return jsonify({"error": f"API request failed: {e}"})
    except Exception as e:
        return jsonify({"error": f"An unexpected error occurred during API call: {str(e)}"})


if __name__ == "__main__":
    app.run(debug=True, port=5000)

