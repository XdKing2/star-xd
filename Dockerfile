FROM node:20-bullseye-slim

WORKDIR /root/star-xd

# Install system dependencies for sharp and other native modules
RUN apt-get update && apt-get install -y \
    ffmpeg \
    imagemagick \
    git \
    python3 \
    make \
    g++ \
    pkg-config \
    libvips-dev \
    libglib2.0-dev \
    libcairo2-dev \
    libpango1.0-dev \
    libjpeg-dev \
    libgif-dev \
    librsvg2-dev \
    libwebp-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy package files
COPY package.json package-lock.json ./

# Install dependencies
RUN npm install --build-from-source

# Copy loader
COPY index.js ./

# Create directories
RUN mkdir -p data session temp assets lib plugins

EXPOSE 5000

CMD ["npm", "run", "start:optimized"]
