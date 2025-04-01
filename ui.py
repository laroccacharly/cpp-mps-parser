# Instructions: 
# in the folder "data", we have a list of folders. 
# Each folder contains a metadata.json. That json has the field "parse_time_seconds". 
# We want to display the distrbution of the parse_time_seconds for all folders in the folder "data".
# Use plotly and streamlit to create a plot that displays the distribution of the parse_time_seconds.

import streamlit as st
import pandas as pd
import plotly.express as px
import os
import json
from pathlib import Path

DATA_DIR = Path("data")
NUM_INSTANCES_TO_FILTER = 40 # Number of instances with highest n_vars to filter out

def load_metadata(data_dir: Path) -> list[dict]:
    """Loads parse_time_seconds and n_vars from metadata.json files in subdirectories."""
    metadata_list = []
    if not data_dir.is_dir():
        st.error(f"Data directory '{data_dir}' not found.")
        return metadata_list

    for item in data_dir.iterdir():
        if item.is_dir():
            metadata_path = item / "metadata.json"
            if metadata_path.is_file():
                try:
                    with open(metadata_path, 'r') as f:
                        metadata = json.load(f)
                        if "parse_time_seconds" in metadata and "n_vars" in metadata:
                            metadata_list.append({
                                "path": str(metadata_path),
                                "parse_time_seconds": float(metadata["parse_time_seconds"]),
                                "n_vars": int(metadata["n_vars"])
                            })
                        else:
                            missing_keys = []
                            if "parse_time_seconds" not in metadata:
                                missing_keys.append("'parse_time_seconds'")
                            if "n_vars" not in metadata:
                                missing_keys.append("'n_vars'")
                            st.warning(f"{', '.join(missing_keys)} not found in {metadata_path}")
                except (json.JSONDecodeError, ValueError, IOError) as e:
                    st.error(f"Error reading or parsing {metadata_path}: {e}")
            # else:
            #     st.warning(f"metadata.json not found in directory: {item}")
    return metadata_list

# --- Streamlit App ---

st.title("MPS File Parse Time Distribution (Excluding Top 40 by Variable Count)")

all_metadata = load_metadata(DATA_DIR)

if not all_metadata:
    st.warning("No metadata found or loaded. Cannot generate plot.")
elif len(all_metadata) <= NUM_INSTANCES_TO_FILTER:
     st.warning(f"Not enough data ({len(all_metadata)} instances) to filter out the top {NUM_INSTANCES_TO_FILTER}. Displaying all data.")
     parse_times = [item["parse_time_seconds"] for item in all_metadata]
else:
    # Sort by n_vars in descending order
    all_metadata.sort(key=lambda x: x["n_vars"], reverse=True)
    
    # Filter out the top NUM_INSTANCES_TO_FILTER
    filtered_metadata = all_metadata[NUM_INSTANCES_TO_FILTER:]
    
    st.info(f"Filtered out {NUM_INSTANCES_TO_FILTER} instances with the highest 'n_vars'. Plotting distribution for the remaining {len(filtered_metadata)} instances.")

    # Extract parse times from the filtered list
    parse_times = [item["parse_time_seconds"] for item in filtered_metadata]

# Proceed only if we have parse_times (either original or filtered)
if 'parse_times' in locals() and parse_times:
    # Create a Pandas DataFrame
    df = pd.DataFrame(parse_times, columns=["Parse Time (seconds)"])

    # Create the histogram using Plotly Express
    fig = px.histogram(
        df,
        x="Parse Time (seconds)",
        nbins=20, # Adjust number of bins as needed
        title=f"Distribution of Parse Times (Filtered - Top {NUM_INSTANCES_TO_FILTER} 'n_vars' removed)",
        labels={"Parse Time (seconds)": "Parse Time (seconds)"},
        opacity=0.8,
    )
    
    fig.update_layout(bargap=0.1)


    # Display the plot in Streamlit
    st.plotly_chart(fig, use_container_width=True)

    st.write("Summary Statistics:")
    st.write(df.describe())

