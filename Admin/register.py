import os 
import sys 
import tkinter as tk
import tkinter.filedialog
from tkinter import ttk
import firebase_admin
from firebase_admin import credentials, db

def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    try:
        # PyInstaller creates a temp folder and stores path in _MEIPASS
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")

    return os.path.join(base_path, relative_path)

Logo = resource_path("Logo.png")

# Initialize Firebase
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': "https://student-register-3249a-default-rtdb.firebaseio.com/"
})

ref = db.reference('Students')


def submit_registration():
    student_number = student_entry.get()
    data = {
        "name" : name_entry.get(),
        "age" : age_entry.get(),
        "phone_number" : phone_entry.get(),
        "fingerprint" : fingerprint_entry.get(),
        "gender" : gender_entry.get()

    }
    
    ref.child(student_number).set(data)

    
    # You can process or save this data as needed
    
    # Clear the entry fields after submission
    name_entry.delete(0, tk.END)
    age_entry.delete(0, tk.END)
    student_entry.delete(0, tk.END)
    phone_entry.delete(0, tk.END)
    fingerprint_entry.delete(0, tk.END)
    gender_entry.delete(0, tk.END)

root = tk.Tk()
root.title("Registration Form")

# Labels
name_label = tk.Label(root, text="Name:")
age_label = tk.Label(root, text="Age:")
student_label = tk.Label(root, text="Student Number:")
phone_label = tk.Label(root, text="Phone Number:")
fingerprint_label = tk.Label(root, text="Fingerprint ID:")
gender_label = tk.Label(root, text="Gender:")

name_label.grid(row=0, column=0)
age_label.grid(row=1, column=0)
student_label.grid(row=2, column=0)
phone_label.grid(row=3, column=0)
fingerprint_label.grid(row=5, column=0)
gender_label.grid(row=6, column=0)

# Entry fields
name_entry = tk.Entry(root)
age_entry = tk.Entry(root)
student_entry = tk.Entry(root)
phone_entry = tk.Entry(root)
fingerprint_entry = tk.Entry(root)
gender_entry = tk.Entry(root)

name_entry.grid(row=0, column=1)
age_entry.grid(row=1, column=1)
student_entry.grid(row=2, column=1)
phone_entry.grid(row=3, column=1)
fingerprint_entry.grid(row=5, column=1)
gender_entry.grid(row=6, column=1)

# Upload button (assuming you want to open a file dialog)
#

# Submit button
submit_button = ttk.Button(root, text="Submit", command=submit_registration)
submit_button.grid(row=7, column=0, columnspan=2)

root.mainloop()
